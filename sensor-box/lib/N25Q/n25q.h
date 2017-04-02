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

#ifndef N25Q_DRIVER
#define N25Q_DRIVER

#include "mbed.h"
#include <string>

//#define DEBUG
//#define TEST
#ifdef DEBUG
#define dbgprintf(...) printf (__VA_ARGS__)
#else
#define dbgprintf(...) (void)0
#endif

#ifdef TEST
#define testprintf(...) printf (__VA_ARGS__)
#else
#define testprintf(...) (void)0
#endif

#define READ_ID_LEN     17
#define PAGE_SIZE 	    256
#define SUBSECTOR_SIZE  4096
#define FLASH_SIZE      8388608

#define WRITE_STATUS_REG_CMD		0x01
#define PAGE_PROGRAM				0x02
#define READ_CMD 					0x03
#define WRITE_DISABLE_CMD 			0x04
#define READ_STATUS_REG_CMD 		0x05
#define WRITE_ENABLE_CMD 			0x06
#define SUBSECTOR_ERASE_CMD			0x20
#define CLEAR_FLAG_STATUS_REG_CMD	0x50
#define READ_FLAG_STATUS_REG_CMD	0x70
#define BULK_ERASE_CMD				0xC7
#define SECTOR_ERASE_CMD			0xD8
#define WRITE_LOCK_REG_CMD  		0xE5
#define READ_LOCK_REG_CMD   		0xE8
#define READ_ID_CMD 				0x9F

class N25Q
{
public:
	N25Q();
	virtual ~N25Q();

	void ReadID(uint8_t * id_string, int length=READ_ID_LEN);
	void ReadDataFromAddress(uint8_t * dataBuffer, int startingAddress, int length);
	void ProgramFromAddress(uint8_t * dataBuffer, int startingAddress, int length);
	void NonBlockingProgramFromAddress(uint8_t * dataBuffer, int startingAddress, int length);
	void WriteEnable(void);
	void WriteDisable(void);

	int  ReadStatusRegister(void);
	void WriteStatusRegister(int status_mask);
	int	 ReadLockRegister(int startingAddress);
	void WriteLockRegister(int startingAddress, int lock_mask);
	void ClearFlagStatusRegister(void);
	int	 ReadFlagStatusRegister(void);

	void SubSectorErase(int startingAddress);
	void SectorErase(int startingAddress);
	void BulkErase(void);
	void NonBlockingBulkErase(void);

	bool isBusy(void);
	bool isSubSectorWritten(int startingAddress);
protected:

private:
	int  SPIReadByte(void);
	void SPIReadNBytes(uint8_t * rxBuffer, int length);
	void SPIInit(void);
	void SlaveSelect(void);
	void SlaveDeSelect(void);

	static SPI * m_SPI;
	static DigitalOut * m_SSEL;
};

#endif
