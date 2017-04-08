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

class N25Q
{
public:
  N25Q(PinName mosi, PinName miso, PinName sclk, PinName ssel);
	virtual ~N25Q();

  static const int READ_ID_LEN               = 17;
  static const int PAGE_SIZE                 = 256;
  static const int SUBSECTOR_SIZE            = 4096;
  static const int FLASH_SIZE                = 8388608;

  static const int WRITE_STATUS_REG_CMD      = 0x01;
  static const int PAGE_PROGRAM              = 0x02;
  static const int READ_CMD                  = 0x03;
  static const int WRITE_DISABLE_CMD         = 0x04;
  static const int READ_STATUS_REG_CMD       = 0x05;
  static const int WRITE_ENABLE_CMD          = 0x06;
  static const int SUBSECTOR_ERASE_CMD       = 0x20;
  static const int CLEAR_FLAG_STATUS_REG_CMD = 0x50;
  static const int READ_FLAG_STATUS_REG_CMD  = 0x70;
  static const int BULK_ERASE_CMD            = 0xC7;
  static const int SECTOR_ERASE_CMD          = 0xD8;
  static const int WRITE_LOCK_REG_CMD        = 0xE5;
  static const int READ_LOCK_REG_CMD         = 0xE8;
  static const int READ_ID_CMD               = 0x9F;

  static const int PROGRAM_ERROR             = 0x10;
  static const int ERASE_ERROR               = 0x08;


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
  bool hasError(int err_type);
  bool hasError();
protected:

private:
	int  SPIReadByte(void);
	void SPIReadNBytes(uint8_t * rxBuffer, int length);
	void SPIInit(void);
	void SlaveSelect(void);
	void SlaveDeSelect(void);

	SPI * m_SPI;
	DigitalOut * m_SSEL;
};

#endif
