/*
 * Copyright [2016] [Riccardo Pozza]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author:
 * Riccardo Pozza <r.pozza@surrey.ac.uk>
 */

#include "n25q.h"

N25Q::N25Q(PinName mosi, PinName miso, PinName sclk, PinName ssel) {
  m_SPI = new SPI(mosi, miso, sclk);
  m_SSEL = new DigitalOut(ssel);
	SPIInit();
}

N25Q::~N25Q() {

}

void
N25Q::ReadID(uint8_t * id_string, int length) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Reading Identification Data\r\n");
	SlaveSelect();
	m_SPI->write(READ_ID_CMD);
	SPIReadNBytes(id_string,length);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

//bool isSubSectorWritten(int startingAddress) {
//	uint8_t dataBuffer[256];
//	ReadDataFromAddress(dataBuffer, startingAddress, PAGE_SIZE);
//	if(dataBuffer[0]& 0xFF)
//		return 0;
//	else
//		return 1;
//}

void
N25Q::ReadDataFromAddress(uint8_t * dataBuffer, int startingAddress, int length) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Reading Data From ");

	SlaveSelect();
	m_SPI->write(READ_CMD);
	dbgprintf("Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI->write(addressByte);
	}
	dbgprintf("\r\n");
	SPIReadNBytes(dataBuffer,length);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void
N25Q::ProgramFromAddress(uint8_t * dataBuffer, int startingAddress, int length){
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Writing Data From");

	ClearFlagStatusRegister();

	WriteEnable();

	SlaveSelect();
	m_SPI->write(PAGE_PROGRAM);
	dbgprintf("Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI->write(addressByte);
	}
	dbgprintf("\r\n");
	dbgprintf("Data Written:\r\n");
	for (int z=0; z<length; z++){
		dbgprintf("%X ", dataBuffer[z]);
		m_SPI->write(dataBuffer[z]);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	while (isBusy()){
	}

	testprintf("Ended!\r\n");
}

void
N25Q::NonBlockingProgramFromAddress(uint8_t * dataBuffer, int startingAddress, int length){
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Writing Data From");

	ClearFlagStatusRegister();

	WriteEnable();

	SlaveSelect();
	m_SPI->write(PAGE_PROGRAM);
	dbgprintf("Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI->write(addressByte);
	}
	dbgprintf("\r\n");
	dbgprintf("Data Written:\r\n");
	for (int z=0; z<length; z++){
		dbgprintf("%X ", dataBuffer[z]);
		m_SPI->write(dataBuffer[z]);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void
N25Q::WriteEnable(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Write Enable\r\n");

	SlaveSelect();
	m_SPI->write(WRITE_ENABLE_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void
N25Q::WriteDisable(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	dbgprintf("Write Disable\r\n");

	SlaveSelect();
	m_SPI->write(WRITE_DISABLE_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

int
N25Q::ReadStatusRegister(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI->write(READ_STATUS_REG_CMD);
	int retval = SPIReadByte();
	SlaveDeSelect();

	testprintf("Ended!\r\n");
	return retval;
}

void
N25Q::WriteStatusRegister(int status_mask) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	WriteEnable();

	SlaveSelect();
	m_SPI->write(WRITE_STATUS_REG_CMD);
	m_SPI->write(status_mask);  // only bits 7..2 (1 and 0 are not writable)
	SlaveDeSelect();

	while (isBusy()){
	}

	testprintf("Ended!\r\n");
}

int
N25Q::ReadLockRegister(int startingAddress) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI->write(READ_LOCK_REG_CMD);
	dbgprintf("Read Lock Register From Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI->write(addressByte);
	}
	dbgprintf("\r\n");
	int retval = SPIReadByte();
	SlaveDeSelect();

	testprintf("Ended!\r\n");
	return retval;
}

void
N25Q::WriteLockRegister(int startingAddress, int lock_mask) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	WriteEnable();

	SlaveSelect();
	m_SPI->write(WRITE_LOCK_REG_CMD);
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		m_SPI->write(addressByte);
	}
	m_SPI->write(lock_mask);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

void
N25Q::ClearFlagStatusRegister(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI->write(CLEAR_FLAG_STATUS_REG_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

int
N25Q::ReadFlagStatusRegister(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	SlaveSelect();
	m_SPI->write(READ_FLAG_STATUS_REG_CMD);
	dbgprintf("Read Flag Status Register ");
	int retval = SPIReadByte();
	dbgprintf("\r\n");
	SlaveDeSelect();

	testprintf("Ended!\r\n");
	return retval;

}

void
N25Q::SubSectorErase(int startingAddress) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	WriteEnable();

	SlaveSelect();
	m_SPI->write(SUBSECTOR_ERASE_CMD);
	dbgprintf("Subsector Erase From Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI->write(addressByte);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	while (isBusy()){
	}

	testprintf("Ended!\r\n");
}

void
N25Q::SectorErase(int startingAddress) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	WriteEnable();

	SlaveSelect();
	m_SPI->write(SECTOR_ERASE_CMD);
	dbgprintf("Sector Erase From Starting Address: ");
	for (int i=2; i>=0; i--){
		int addressByte = (startingAddress >> 8*i) & 0xFF;
		dbgprintf("%X ", addressByte);
		m_SPI->write(addressByte);
	}
	dbgprintf("\r\n");
	SlaveDeSelect();

	while (isBusy()){
	}

	testprintf("Ended!\r\n");
}

void
N25Q::BulkErase(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	WriteEnable();

	SlaveSelect();
	dbgprintf("Bulk Erase!\r\n");
	m_SPI->write(BULK_ERASE_CMD);
	SlaveDeSelect();

	while (isBusy()){
	}

	testprintf("Ended!\r\n");
}

void
N25Q::NonBlockingBulkErase(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	WriteEnable();

	SlaveSelect();
	dbgprintf("Bulk Erase!\r\n");
	m_SPI->write(BULK_ERASE_CMD);
	SlaveDeSelect();

	testprintf("Ended!\r\n");
}

bool
N25Q::isBusy(void) {
	testprintf("\r\nEntering %s ...", __PRETTY_FUNCTION__);

	int retval = ReadStatusRegister();
	retval &= 0x01;

	if (retval){
		testprintf("Ended!\r\n");
		return true;
	}

	testprintf("Ended!\r\n");
	return false;
}

bool
N25Q::hasError(int err_type) {
  int flag = ReadFlagStatusRegister();
  return flag & err_type;
}

bool
N25Q::hasError() {
  int flag = ReadFlagStatusRegister();
  return (flag & ERASE_ERROR) || (flag & PROGRAM_ERROR);
}

int
N25Q::SPIReadByte(void) {
	int retval = m_SPI->write(0x00);
	dbgprintf(" %X ", retval);
	return retval;
}

void
N25Q::SPIReadNBytes(uint8_t * rxBuffer, int length) {
	dbgprintf("Response:\r\n");
	for (int i=0; i<length; i++){
		rxBuffer[i] = m_SPI->write(0x00);
		dbgprintf("%X ", rxBuffer[i]);
	}
	dbgprintf("\r\n");
}

void
N25Q::SPIInit(void) {
	dbgprintf("Initialising SPI 8bit, 25MHz, CPOL = 1, CPHA = 1\r\n");
	SlaveDeSelect();
	m_SPI->format(8,3); // CPOL = 1, CPHA = 1
	m_SPI->frequency(25000000); // 25MHz
}

void
N25Q::SlaveSelect(void) {
	m_SSEL->write(0);
}

void
N25Q::SlaveDeSelect(void) {
	m_SSEL->write(1);
}

