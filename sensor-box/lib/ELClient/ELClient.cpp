// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClient.h"
#include "mbed.h"
#include "rtos/rtos.h"
#include "Timer.h"

#define SLIP_END  0300        // indicates end of packet
#define SLIP_ESC  0333        // indicates byte stuffing
#define SLIP_ESC_END  0334    // ESC ESC_END means END data byte
#define SLIP_ESC_ESC  0335    // ESC ESC_ESC means ESC data byte

//===== Input

// Process a received SLIP message
ELClientPacket* ELClient::Process(void) {

  osEvent evt = _fragment_queue.get(1);
  if (evt.status != osEventMessage)
    return NULL;
  ELFragment * p = (ELFragment*)evt.value.p;
  fragment.dataLen = p->dataLen;
  memcpy(fragment.buf, p->buf, p->dataLen);
  delete p->buf;
  delete p;

  // verify CRC
  uint16_t crc = crc16Data(fragment.buf, fragment.dataLen-2, 0);
  uint16_t resp_crc = *(uint16_t*)(fragment.buf+fragment.dataLen-2);
  // for(int i=0; i<fragment.dataLen-2; i++)
  //   printf("%02X", fragment.buf[i]);
  // printf("\nELC: CRC[%04X] = %04X (%04X)\n", fragment.dataLen, crc, resp_crc);
  if (crc != resp_crc) {
    for(int i=0; i<fragment.dataLen-2; i++)
      printf("%02X", fragment.buf[i]);
    printf("\nELC: Invalid CRC, %04X != %04X (REF)\n", crc, resp_crc);
    return NULL;
  }

  ELClientPacket* packet = (ELClientPacket*)fragment.buf;
  // dispatch based on command
  if (packet->cmd == CMD_RESP_V) {
    // value response
    return packet;
  } else if (packet->cmd == CMD_RESP_CB) {
    FP<void, void*> *fp;
    fp = (FP<void, void*>*)packet->value;
    if (fp->attached()) {
      ELClientResponse resp(packet);
      (*fp)(&resp);
    }
  } else {
    // command (NOT IMPLEMENTED)
    // _debug->printf("CMD??\n");
  }
  return NULL;
}

// Read all characters available on the serial input and process any messages that arrive, but
// stop if a non-callback response comes in
void ELClient::Recieve() {
  int value;
  while (_serial->readable()) {
    value = _serial->getc();
    if (value == SLIP_ESC) {
      _proto.isEsc = 1;
    } else if (value == SLIP_END) {
      if (_proto.dataLen >= 8){
        ELFragment * f = new ELFragment();
        f->dataLen = _proto.dataLen;
        f->buf = new uint8_t[_proto.dataLen];
        memcpy(f->buf, _proto.buf, _proto.dataLen);
        _fragment_queue.put(f);
      }
      _proto.dataLen = 0;
      _proto.isEsc = 0;
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
  _serial->attach(callback(this, &ELClient::Recieve));
  fragment.buf = f_buf;
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
//  while (millis() - wait < timeout) {
  while (timer.read_ms() < timeout) {
    ELClientPacket *packet = Process();
    if (packet != NULL) {
        timer.stop();
        return packet;
    }
    Thread::wait(1);
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
    if (packet->value == (uint32_t)&wifiCb) { 
      printf("SYNC!\n\r"); 
      return true;
    }
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
