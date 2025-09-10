#ifndef FDC_H
#define FDC_H

#include <stdio.h>
#include <stdint.h>
 
enum FDC_Msr_Flags
{
	// The FDD are in seek mode

	FDC_MSR_FDD_0_SEEK = 0x1,
	FDC_MSR_FDD_1_SEEK = 0x2,
	FDC_MSR_FDD_2_SEEK = 0x4,
	FDC_MSR_FDD_3_SEEK = 0x8,
	FDC_MSR_NON_DMA_MODE = 0x20, // Transitioning from 1 to 0 indicates execution phase ended
	FDC_MSR_DATA_IO = 0x40, // Indications direction of data transfer. 1 = Data I/O -> CPU (read), 0 = CPU -> Data I/O (write)
	FDC_MSR_REQUEST_FOR_MASTER = 0x80, // Indicates data i/o is ready to send/receive data. This and Data IO port should be used to perform the (command phase) hadnshasking functions.
};

enum FDC_Dor_Flags
{
	// The FDD are in seek mode

	FDC_DOR_DRIVE = 0x3,
	FDC_DOR_RESET_OFF = 0x4,
	FDC_DOR_DMA_IRQ_ON = 0x8,
	FDC_DOR_MOTOR_A_ON = 0x10,
	FDC_DOR_MOTOR_B_ON = 0x20,
	FDC_DOR_MOTOR_C_ON = 0x40,
	FDC_DOR_MOTOR_D_ON = 0x80,
};

enum FDC_Port
{
	FDC_PORT_MSR = 0x3F4, // R
	FDC_PORT_DATA_IO = 0x3F5, // R/W
	FDC_PORT_DOR = 0x3F2,
};

enum FDC_Data_Commands
{
	FDC_DATA_COMMAND_SPECIFY = 0x3,
	FDC_DATA_COMMAND_RECALIBRATE = 0x7,
	FDC_DATA_COMMAND_SENSE_INTERRUPT = 0x8,
	FDC_DATA_COMMAND_SEEK = 0xF,
};

enum FDC_ST0 // Used in st0 
{
	FDC_ST0_INT_CODE = 0xC0,
	FDC_ST0_SK_END = 0x20,
	FDC_ST0_EQUIPMENT_CHECK = 0x10,
	FDC_ST0_NOT_READY = 0x8,
	FDC_ST0_HEAD_ADDRESS = 0x4,
	FDC_ST0_DRIVE_SELECT
};

// intel 8272 floppy disk controller.
// In non-dma mode, it will fire irq 6 when there is data to be received/written
// Supports up to four floppies (only one on this emulator)
// There are 15 commands which the 8272 will execute, Each command involve a certain
// number of bytes to specify the operation the processor wants the FDC to do.
// Read/Write Data, Read ID, Read/Write deleted data, Read a track, Scan equal, Scan high/equal, Scan low/equal
// Specify, Seek, Format a trtack, recalibrate (which is go back to track 0), sense interrupt status, sense drive status
// Data register is actually a stack, read from once at atime and stores data, commands, aparameters, and fdd status information.
// Data bytes are read out of, or written into, the data register in order to program or obtain the results after execution fof a commd. 
// The status register may only be read and is used to facilitate the transfer of data between the processor and 8272.
// MSR should be read in command and result phase, continually being checked (B6 and 7)

typedef struct FDD
{
	FILE* floppy1;

	uint8_t cylinder;
	_Bool head; // its either 1 or 0 lmfao
	uint8_t sector;
} FDD;

typedef struct FDC
{	FDD* selected_fdd;
	FDD fdd[1];
	uint8_t st[4];
	uint8_t data_read[9];
	int data_read_current;
	int data_read_bytes;
	uint8_t active_command;
	uint8_t command_step;
	uint8_t msr;
} FDC;
#endif