// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClient.h"
#include "mbed.h"
#include "Timer.h"
#include "WString.h"

#define SLIP_END  0300        // indicates end of packet
#define SLIP_ESC  0333        // indicates byte stuffing
#define SLIP_ESC_END  0334    // ESC ESC_END means END data byte
#define SLIP_ESC_ESC  0335    // ESC ESC_ESC means ESC data byte

//===== Input

// Process a received SLIP message
ELClientPacket* ELClient::protoCompletedCb(void) {
  // the packet starts with a ELClientPacket
  ELClientPacket* packet = (ELClientPacket*)_proto.buf;
//  if (_debugEn) {
//    printf("ELC: got ");
//    printf("%d", _proto.dataLen);
//    printf(" @");
////    _debug->printf("%d", (uint32_t)_proto.buf, 16);
//    printf("%d", (uint32_t)_proto.buf);
//    printf(": ");
////    _debug->printf(packet->cmd, 16);
//    printf("%d", packet->cmd);
//    printf(" ");
////    _debug->printf(packet->value, 16);
//    printf("%d", packet->value);
//    printf(" ");
////    _debug->printf(packet->argc, 16);
//    printf("%d", packet->argc);
//    for (uint16_t i=8; i<_proto.dataLen; i++) {
//      printf(" ");
////      _debug->printf(*(uint8_t*)(_proto.buf+i), 16);
//      printf("%d", *(uint8_t*)(_proto.buf+i));
//    }
//    printf("\n\r");
//  }

  // verify CRC
  uint16_t crc = crc16Data(_proto.buf, _proto.dataLen-2, 0);
  uint16_t resp_crc = *(uint16_t*)(_proto.buf+_proto.dataLen-2);
  if (crc != resp_crc) {
    printf("ELC: Invalid CRC, %d, %d\n\r", crc, resp_crc);
//    return NULL;
  }

  // dispatch based on command
  if (packet->cmd == CMD_RESP_V) {
    // value response
//    printf("RESP_V: ");
//    printf("%d\n\r", packet->value);
    return packet;
  } else if (packet->cmd == CMD_RESP_CB) {
    FP<void, void*> *fp;
    // callback reponse
    _debug->printf("RESP_CB: ");
    _debug->printf("%d", packet->value);
    _debug->printf(" ");
    _debug->printf("%d\n", packet->argc);
    fp = (FP<void, void*>*)packet->value;
    if (fp->attached()) {
      ELClientResponse resp(packet);
      (*fp)(&resp);
    }
    return NULL;
  } else {
    // command (NOT IMPLEMENTED)
    _debug->printf("CMD??\n");
    return NULL;
  }
}

// Read all characters available on the serial input and process any messages that arrive, but
// stop if a non-callback response comes in
ELClientPacket *ELClient::Process() {
  int value;
  while (_serial->readable()) {
    value = _serial->getc();
//    printf("value is: %d\n\r", value);
    if (value == SLIP_ESC) {
      _proto.isEsc = 1;
    } else if (value == SLIP_END) {
      ELClientPacket* packet = _proto.dataLen >= 8 ? protoCompletedCb() : 0;
      _proto.dataLen = 0;
      _proto.isEsc = 0;
      if (packet != NULL) {
//        printf("packet returned \n\r");
        return packet;
      }
    } else {
      if (_proto.isEsc) {
        if (value == SLIP_ESC_END) value = SLIP_END;
        if (value == SLIP_ESC_ESC) value = SLIP_ESC;
        _proto.isEsc = 0;
      }
      if (_proto.dataLen < _proto.bufSize) {
        _proto.buf[_proto.dataLen++] = value;
      }
    }
  }
//  printf("returned null value!!!!\n\r");
  return NULL;
}

//===== Output

// Write a byte to the output stream and perform SLIP escaping
void ELClient::write(uint8_t data) {
  switch (data) {
  case SLIP_END:
    _serial->putc(SLIP_ESC);
    _serial->putc(SLIP_ESC_END);
    break;
  case SLIP_ESC:
    _serial->putc(SLIP_ESC);
    _serial->putc(SLIP_ESC_ESC);
    break;
  default:
    _serial->putc(data);
  }
}

// Write some bytes to the output stream
void ELClient::write(void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;
  while (len--)
    write(*d++);
}

// Start a request. cmd=command, value=address of callback pointer or first arg,
// argc=additional argument count
void ELClient::Request(uint16_t cmd, uint32_t value, uint16_t argc) {
  crc = 0;
  _serial->putc(SLIP_END);
  write(&cmd, 2);
  crc = crc16Data((unsigned const char*)&cmd, 2, crc);

  write(&argc, 2);
  crc = crc16Data((unsigned const char*)&argc, 2, crc);

  write(&value, 4);
  crc = crc16Data((unsigned const char*)&value, 4, crc);
}

// Append a block of data as an argument to the request
void ELClient::Request(const void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;

  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  for (uint16_t l=len; l>0; l--) {
    write(*d);
    crc = crc16Add(*d, crc);
    d++;
  }

//  printf("data length: %d \n\r", len);

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

// Append a block of data located in flash as an argument to the request
void ELClient::Request(const __FlashStringHelper* data, uint16_t len) {
  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  PGM_P p = reinterpret_cast<PGM_P>(data);
  for (uint16_t l=len; l>0; l--) {
    uint8_t c = pgm_read_byte(p++);
    write(c);
    crc = crc16Add(c, crc);
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

// Append the final CRC to the request and finish the request
void ELClient::Request(void) {
  write((uint8_t*)&crc, 2);
  _serial->putc(SLIP_END);
}

//===== Initialization

void ELClient::init() {
  _proto.buf = _protoBuf;
  _proto.bufSize = sizeof(_protoBuf);
  _proto.dataLen = 0;
  _proto.isEsc = 0;
}

ELClient::ELClient(Serial* serial) :
_serial(serial) {
  _debugEn = false;
  init();
}

ELClient::ELClient(Serial* serial, Serial* debug) :
_debug(debug), _serial(serial) {
  _debugEn = true;
  init();
}

void ELClient::DBG(const char* info) {
  if (_debugEn) _debug->printf("%s\n", info);
}

//===== Responses

// Wait for a response for a given timeout
ELClientPacket *ELClient::WaitReturn(uint32_t timeout) {
//  uint32_t wait = millis();
  Timer timer;
  timer.start();
//  printf("getting value 111 !!!\n\r");
//  while (millis() - wait < timeout) {
  while (timer.read_ms() < timeout) {
    ELClientPacket *packet = Process();
    if (packet != NULL) {
        timer.stop();
//        printf("getting value 222!!!\n\r");
        return packet;
    }
  }
  timer.stop();
  return NULL;
}

//===== CRC helper functions

uint16_t ELClient::crc16Add(unsigned char b, uint16_t acc)
{
  acc ^= b;
  acc = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}

uint16_t ELClient::crc16Data(const unsigned char *data, uint16_t len, uint16_t acc)
{
  for (uint16_t i=0; i<len; i++)
    acc = crc16Add(*data++, acc);
  return acc;
}

//===== Basic requests built into ElClient

bool ELClient::Sync(uint32_t timeout) {
  printf("start sync ...\n\r");
  // send sync request
  Request(CMD_SYNC, (uint32_t)&wifiCb, 0);
  Request();
  // empty the response queue hoping to find the wifiCb address
  ELClientPacket *packet;
  while ((packet = WaitReturn(timeout)) != NULL) {
    if (packet->value == (uint32_t)&wifiCb) { printf("SYNC!\n\r"); return true; }
    printf("BAD: ");
    printf("%d\n\r", packet->value);
  }
  // doesn't look like we got a real response
  return false;
}

void ELClient::GetWifiStatus(void) {
  Request(CMD_WIFI_STATUS, 0, 0);
  Request();
}
