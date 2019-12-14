//           ****************************************************
//          **   Processor      : STM32F107VCT6                   **
//         ***   Frequency      : 8MHz ExtClock 72MHZ CPU Clock   ***
//        ****   AUTHOR         : Mahdi Dadashi                   ****
//       *****   STM32CubeMX    : V5.40                           *****
//      ******   STM32CubeF1    : V1.8.0                          ******
//       *****   Compiler       : KEIL uVision V5.26              *****
//        ****   Instagram      : instagram.com/mahdidadashi65/   ****
//         ***   Telegram       : t.me/mahdidadashi65/            ***
//          **   Github         : github.com/mahdidadashi65/      **
//           ****************************************************
//.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._
//.-''-._.-''-._.-''                                 ''-._.-''-._.-''-._.-''-._
//.-''-._.-''-._.-''      www.mahdidadashi.ir        ''-._.-''-._.-''-._.-''-._
//.-''-._.-''-._.-''                                 ''-._.-''-._.-''-._.-''-._
//.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._.-''-._


/*
 * Ch376msc.h
 *
 *  Created on: Feb 25, 2019
 *      Author: György Kovács
 *  Copyright (c) 2019, György Kovács
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 ******************************************************
 * Versions:                                          *
 * ****************************************************
 * v1.4.0 Sep 26, 2019 
 * 	- new functions
 *   	getTotalSectors() - returns a unsigned long number, total sectors on the drive
 *   	getFreeSectors() - returns a unsigned long number, free sectors on the drive
 *   	getFileSystem() - returns a byte number, 0x01-FAT12, 0x02-FAT16, 0x03-FAT32
 * 	- updated example files with a new functions
 * 	- new example file, seraching the oldest/newest file on the flash drive
 * **************************************************** 
 * 	v1.3 Sep 17, 2019
 * 		-bug fix for moveCursor issue #3  
 *	https://github.com/djuseeq/Ch376msc/issues/3
 * ****************************************************
 *  v1.2 Apr 24, 2019
 *  	-bug fix for timing issue on higher SPI clock
 *  	 datasheet 7.3 Time Sequence table (TSC)
 ******************************************************
 *  v1.2 Apr 20, 2019
 *  	-extended with SPI communication
 ******************************************************
 *	v1.1 Feb 25, 2019
 *		-initial version with UART communication
 ******************************************************
 */

#ifndef Ch376msc_H_
#define Ch376msc_H_

#include "main.h"
#include <stdbool.h>
//#include "CommDef.h"
//#include <Stream.h>
//#include <SPI.h>


#define TIMEOUT 2000 // waiting for data from CH
#define SPICLKRATE 125000 //Clock rate 125kHz				SystemClk  DIV2  MAX
						// max 8000000 (8MHz)on UNO, Mega (16 000 000 / 2 = 8 000 000)


#define MODE_HOST_0 0x05
#define MODE_HOST_1 0x06
#define MODE_HOST_2 0x07
#define CURSORBEGIN 0x00000000
#define CURSOREND 0xFFFFFFFF
#define SECTORSIZE 512

////////////Commands/////////
#define CMD_GET_IC_VER 0x01
	//Result: 1 byte in data register, version number | 0x40
#define CMD_SET_BAUDRATE 0x02
	//Serial port speed
#define CMD_ENTER_SLEEP 0x03
	//Put device into sleep mode.
#define CMD_SET_USB_SPEED 0x04
/* The command sets the USB bus speed. The command requires a data input for selecting USB bus speed, corresponding to 00H
 * 12Mbps full mode, 01H at full speed corresponding to 1.5Mbps mode (non-standard mode),
 * 02H 1.5Mbps corresponding to the low speed mode. CH376 USB bus speed of 12Mbps full-speed mode by default,
 * and execution will be automatically restored to full speed 12Mbps mode after CMD_SET_USB_MODE command sets USB mode.
 */
#define CMD_RESET_ALL 0x05
	//Need to wait 35ms before device is ready again
#define CMD_CHECK_EXIST 0x06
	//Test that the interface exists and works.
	//Input: one data byte
	//Output: !input
#define CMD_SET_SD0_INT 0x0b // use SPI MISO pin as INT input
#define CMD_SET_RETRY 0x0b
// Input: 0x25, setup retry times
//  bit7=1 for infinite retry, bit3~0 retry times
#define CMD_GET_FILE_SIZE 0x0c
	//Input: 0x68
	//Output: file length on 4 bytes
#define CMD_SET_USB_ADDRESS 0x13
/*  This command sets the USB device address.
 *  The command requires a data input for selecting the USB device address is operated. After a reset or a USB device is
 *  connected or disconnected, the USB device address is always 00H, 00H and the
 *  microcontroller through a USB device Default address communication. If the microcontroller through a
 *  standard USB requests an address set up USB device, then you must also set the same USB device address by this command,
 *  in order to address the new CH376 USB device communication. //Chinese doc
 */
#define CMD_SET_USB_MODE 0x15
/*	Switch between different USB modes.
	Input:
		00: invalid device mode (reset default)
		01: usb device, "peripheral firmware"
		02: usb device, "inner firmware"
		03: SD host, manage SD cards
		04: invalid usb host
		05: usb host, don't generate SOF
		06: usb host, generate SOF
		07: usb host, bus reset
	Output:
		0x51: success
		0x5F: failure
*/
#define CMD_GET_STATUS 0x22
	//Get interrupt status after an interrupt was triggered.
	//Output: interrupt status (see below)
#define CMD_RD_USB_DATA0 0x27
/*	Read data from interrupt port, or USB receive buffer.
	Output: length + data
*/
#define CMD_WR_USB_DATA 0x2c
	//Write data to transfer buffer
	//Input: length + data
#define CMD_WR_REQ_DATA 0x2d
/*	Write requested data
	Used when writing to files
	Output (before input!): length of chunk to write
	Input: data to fill the requested length
*/
#define CMD_WR_OFS_DATA 0x2e
	//Write data to buffer with offset
	//Input: offset, length, data
#define CMD_SET_FILE_NAME 0x2f
/*	Set file or directory name for filesystem operations
	Input: null-terminated string
	The command accepts at most 14 characters. File name must start with '/'.
	Special values:
	"": do not open anything
	"*": list every files
	"/": open root directory
	"/FOO.TXT": file in root directory
	"FOO.TXT": file in current directory
*/
	//These commands have no direct output, instead they trigger an interrupt when done running.

#define CMD_DISK_CONNECT 0x30
	//Wait for USB mass storage to be connected
	//Interrupt with USB_INT_SUCCESS if drive is ready.
#define CMD_DISK_MOUNT 0x31
	//Mount detected USB drive.
	//Triggers USB_INT_SUCCESS and returns 36 byte drive identifier in interrupt buffer.
#define CMD_FILE_OPEN 0x32
/*	Open a file or directory.
	Can also return ERR_MISS_FILE if the file is not found.
*/
#define CMD_FILE_ENUM_GO 0x33
/*	Enumerate next file
	Used for reading directory catalog, get next FAT32 entry
	Use CMD_SET_FILE_NAME with a pattern (eg. "/ *" to list all files in root dir).
	Then use FILE_OPEN to get the first matching file.
	Interrupt status will be USB_INT_DISK_READ, data will be the FAT32 directory entry
	Then use this command to move on to the next matching file until the interrupt is ERR_MISS_FILE.
*/
#define CMD_FILE_CREATE 0x34
/*	Create a file (or truncate an existing file).
	The file must be open (you will get ERR_MISS_FILE) before creating.
	The default date is 2004/1/1 and length is 1 byte.
	USe DIR_INFO_READ and DIR_INFO_SAVE to edit the directory entry.
*/
#define CMD_FILE_ERASE 0x35
/*	Delete a file.
	Make sure the current file is closed first or it will also be deleted!
	Use SET_FILE_NAME then CMD_FILE_ERASE
*/
#define CMD_FILE_CLOSE 0x36
/*	Close an open file.
 *	Input: 1 to update file length, 0 to leave it unchanged
*/
#define CMD_DIR_INFO_READ 0x37
/*	Read directory info
 *	Input one byte which is the id of the file to get info from (in the current dir). Only the first
 *	16 entries can be accessed this way!
 *	Otherwise, first open the file then query for entry 0xFF. The FAT entry for the currently open
 *	file will be returned.
 *	The data is returned in the interrupt stream.
*/
#define CMD_DIR_INFO_SAVE 0x38
/*	Update directory info
 *	You can modify the directory entry using WR_OFS_DATA and then write it again using this command.
*/
#define CMD_BYTE_LOCATE 0x39
/*	Seek to position in file
 *	Input: 4 byte file offset
 *	Returns USB_INT_SUCCESS with new (absolute) offset or FFFFFFFF if reached end of file.
 *	Moving to FFFFFFFF actually seeks to the end of the file (to write in append mode)
*/
#define CMD_BYTE_READ 0x3a
/*	Read from file
 *	Data is returned in chunks of 255 bytes max at a time as interrupt data, then BYTE_RD_GO must be
 *	used to get next chunk (as long as the interrupt status is USB_INT_DISK_READ).
 *	If the pointer becomes USB_INT_SUCCESS before the requested number of bytes has been read, it
 *	means the EOF was reached.
 *	Input: number of bytes to read (16 bit)
*/
#define CMD_BYTE_RD_GO 0x3b
	//Get next chunk of data after BYTE_READ
#define CMD_BYTE_WRITE 0x3c
/*	Write to file
 *	Triggers interrupt USB_INT_DISK_WRITE. MCU should ask how much bytes to write using WR_REQ_DATA
 *	and send the bytes. Operation is finished when the interrupt is USB_INT_SUCCESS.
 *	Size in FAT will be updated when closing the file.
*/
#define CMD_BYTE_WR_GO 0x3d
	//Continue write operation, after a WR_REQ_DATA if the interrupt is not INT_SUCCESS yet.
#define CMD_DISK_CAPACITY 0x3e
	//Get the number of sectors on disk (interrupt return, 4 bytes).
#define CMD_DISK_QUERY 0x3f
/*	Get the info about the FAT partition via interrupt data:
 *	4 bytes: total number of sectors
 *	4 bytes: number of free sectors
 *	1 byte: partition type
*/
#define CMD_DIR_CREATE 0x40
/*	Create and open a directory (name must be set using SET_FILE_NAME).
 *	Open an already existing directory (does not truncate)
 *	Returns ERR_FOUND_NAME if the name exists but is a file
 *	Can only support a single level of directories?
 *	As with FILE_CREATE, the FAT entry can be edited (default values are the same except size is 0 and
 *	directory attribute is set)
*/
#define CMD_SET_ADDRESS 0x45
/*	The command is to set the USB control transfer command address. The command requires a data input,
 * 	a new USB device address is specified, the effective address is 00H ~ 7FH.
 * 	This command is used to simplify the standard USB requests SET_ADDRESS,
 * 	CH376 interrupt request to the MCU after the command is completed,
 * 	if the interrupt status is USB_INT_SUCCESS, then the command is executed successfully.//Chinese doc
 */
#define CMD_GET_DESCR 0x46
/* This command is to obtain a control transfer command descriptor. This command needs to input data specifying
 * the type of the descriptor to be acquired, effective type is 1 or 2, corresponding respectively to DEVICE device descriptors
 * and CONFIGURATION configuration descriptor, wherein the configuration descriptor further includes an interface descriptor,
 * and endpoint descriptor symbol. This command is used to simplify USB request GET_DESCRIPTOR,
 * CH376 interrupt request to the microcontroller upon completion of the command, if the interrupt status is USB_INT_SUCCESS,
 * then the command is executed successfully, the device can be acquired by CMD_RD_USB_DATA0 command descriptor data.
 * Since the control of the transmission buffer CH376 only 64 bytes, when the descriptor is longer than 64 bytes,
 *  the returning operation state CH376 USB_INT_BUF_OVER, for the USB device, the device can be controlled by CMD_ISSUE_TKN_X command transmission process itself.
 */
#define CMD_SET_CONFIG 0x49
/* The command set is a control transfer instruction USB configuration. The command requires a data input,
 * to specify a new USB configuration values, configuration 0,configuration is canceled, or should the configuration descriptor from the USB device.
 * This command is used to simplify the standard USB requests SET_CONFIGURATION,CH376 interrupt request to the MCU after the command is completed,
 * if the interrupt status is USB_INT_SUCCESS, then the command is executed successfully.//Chinese doc
 */
#define CMD_AUTO_CONFIG 0x4D
/* This command is used to automatically configure the USB device does not support SD card.
 * This command is used to simplify the initialization step ordinary USB device corresponds GET_DESCR, SET_ADDRESS,
 * SET_CONFIGURATION like plurality of command sequences. CH376 After completion of the command request interrupt
 * to the microcontroller, if the interrupt status is USB_INT_SUCCESS, then the command is executed successfully.
 */
#define CMD_ISSUE_TKN_X 0x4E
/* The command used to trigger data transfers with the USB devices.
 * The second parameter tells we are performing a control transfer (0x80), on endpoint 0 (the 4 high bits).
 * An USB device has several endpoints, which are like independent communication channels.
 * Endpoint 0 is used for control transfers, specific commands to configure the device.
 */


/* /////////Answers from CH376///////
 *
 *	Interrupt status
 *	================
 *
 *	Bit 6 of the status port is 0 when an interrupt is pending.
 *	As read from command 0x22, status of interrupts (also clears the interrupt)
 *	00 to 0F is for USB device mode (see CH372 docs)
 *
 *	0x2*, 0x3*: usb device error
 *	bit 4: parity valid (if the bit is 0 data may be corrupt)
 *	Low nibble:
 *		0xA: NAK
 *		0xE: stalled transfer
 *		xx00: timeout
 *		other: PID of device
*/

///////////////////////////////////////////////////////////////////////////////////
#define ANSW_RET_SUCCESS 0x51		//Operation successful

#define ANSW_USB_INT_SUCCESS 0x14	//Operation successful, no further data
#define ANSW_USB_INT_CONNECT 0x15	//New USB device connected
#define ANSW_USB_INT_DISCONNECT 0x16//USB device unplugged!

#define ANSW_USB_INT_USB_READY 0x18	//Device is ready
#define ANSW_USB_INT_DISK_READ 0x1d	//Disk read operation
#define ANSW_USB_INT_DISK_WRITE 0x1e//Disk write operation


#define ANSW_RET_ABORT 0x5F			//Operation failure
#define ANSW_USB_INT_DISK_ERR 0x1f	//USB storage device error
#define ANSW_USB_INT_BUF_OVER 0x17	//Buffer overflow
#define ANSW_ERR_OPEN_DIR 0x41		//Tried to open a directory with FILE_OPEN
#define ANSW_ERR_MISS_FILE 0x42		//File not found
#define ANSW_ERR_FOUND_NAME 0x43
#define ANSW_ERR_DISK_DISCON 0x82	//Disk disconnected
#define ANSW_ERR_LARGE_SECTOR 0x84	//Sector size is not 512 bytes
#define ANSW_ERR_TYPE_ERROR 0x92	//Invalid partition type, reformat drive
#define ANSW_ERR_BPB_ERROR 0xa1		//Partition not formatted
#define ANSW_ERR_DISK_FULL 0xb1		//Disk full
#define ANSW_ERR_FDT_OVER 0xb2		//Directory full
#define ANSW_ERR_FILE_CLOSE 0xb4	//Attempted operation on closed file


//class Ch376msc {
//public:
	/////////////Constructors////////////////////////
	//Ch376msc(HardwareSerial &usb, uint32_t speed);//HW serial
	//Ch376msc(Stream &sUsb);// SW serial
	//Ch376msc(uint8_t spiSelect, uint8_t busy);//SPI with MISO as Interrupt pin
//	Ch376msc(uint8_t spiSelect, uint8_t busy, uint8_t intPin);
//	virtual ~Ch376msc(void);//destructor
	////////////////////////////////////////////////
	void Ch376msc_init(void);

	uint8_t mount(void);
	uint8_t dirInfoSave(void);
	uint8_t Ch376msc_openFile(void);
	uint8_t Ch376msc_closeFile(void);
	uint8_t moveCursor(uint32_t position);
	uint8_t deleteFile(void);
	uint8_t pingDevice(void);
	uint8_t listDir(void);
	uint8_t Ch376msc_readFile(char* buffer, uint8_t b_num);
	uint8_t writeFile(char* buffer, uint8_t b_num);
	bool Ch376msc_checkDrive(void); // check is it any interrupt message came from CH
	//bool listDir(char (*nameBuff)[12],uint32_t *sizeBuff,uint8_t listElements); //376_7
	//void reset(void);

//set/get

	//uint32_t getComSpeed(void);
	uint32_t Ch376msc_getFreeSectors(void);
	uint32_t Ch376msc_getTotalSectors(void);
	uint32_t Ch376msc_getFileSize(void);
	uint16_t Ch376msc_getYear(void);
	uint16_t Ch376msc_getMonth(void);
	uint16_t Ch376msc_getDay(void);
	uint16_t Ch376msc_getHour(void);
	uint16_t Ch376msc_getMinute(void);
	uint16_t Ch376msc_getSecond(void);
	uint8_t Ch376msc_getStatus(void);
	uint8_t Ch376msc_getFileSystem(void);
	char* Ch376msc_getFileName(void);
	char* Ch376msc_getFileSizeStr(void);
	bool Ch376msc_getDeviceStatus(void); // usb device mounted, unmounted
	bool Ch376msc_getCHpresence(void);

	void Ch376msc_setFileName(const char* filename);
	void Ch376msc_setYear(uint16_t year);
	void Ch376msc_setMonth(uint16_t month);
	void Ch376msc_setDay(uint16_t day);
	void Ch376msc_setHour(uint16_t hour);
	void Ch376msc_setMinute(uint16_t minute);
	void Ch376msc_setSecond(uint16_t second);

//private:
	//
	//uint8_t read(void);
	void write(uint8_t data);
	void print(const char str[]);
	void spiReady(void);
	void spiBeginTransfer(void);
	void spiEndTransfer(void);
	uint8_t spiWaitInterrupt(void);
	uint8_t spiReadData(void);

	uint8_t getInterrupt(void);
	uint8_t fileEnumGo(void);
	uint8_t byteRdGo(void);
	uint8_t fileCreate(void);
	uint8_t byteWrGo(void);
	uint8_t reqByteRead(uint8_t a);
	uint8_t reqByteWrite(uint8_t a);
	uint8_t readSerDataUSB(void);
	uint8_t writeDataFromBuff(char* buffer);
	uint8_t readDataToBuff(char* buffer);
	uint8_t dirInfoRead(void);
	uint8_t setMode(uint8_t mode);

	void rdFatInfo(void);
	void setSpeed(void);
	void sendCommand(uint8_t b_parancs);
	void Ch376msc_sendFilename(void);
	void writeFatData(void);
	void constructDate(uint16_t value, uint8_t ymd);
	void constructTime(uint16_t value, uint8_t hms);
	void rdDiskInfo(void);




//};//end class

#endif /* Ch376msc_H_ */

