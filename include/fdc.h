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
	FDC_MSR_BUSY = 0x10,// Transitioning from 1 to 0 indicates execution phase ended
	FDC_MSR_NON_DMA_MODE = 0x20, 
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
	FDC_DATA_COMMAND_READ = 0x6,
	FDC_DATA_COMMAND_RECALIBRATE = 0x7,
	FDC_DATA_COMMAND_SENSE_INTERRUPT = 0x8,
	FDC_DATA_COMMAND_READ_ID = 0xA,
	FDC_DATA_COMMAND_SEEK = 0xF,
};

enum FDC_ST0 // Used in st0 
{
	FDC_ST0_INT_CODE = 0xC0,
	FDC_ST0_SK_END = 0x20,
	FDC_ST0_EQUIPMENT_CHECK = 0x10,
	FDC_ST0_NOT_READY = 0x8,
	FDC_ST0_HEAD_ADDRESS = 0x4,
	FDC_ST0_DRIVE_SELECT = 0x3,
};

enum FDC_ST1 // Used in st1
{
	FDC_ST1_EN_CYLINDER = 0x80,
	FDC_ST1_DATA_ERROR = 0x20,
	FDC_ST1_OVER_RUN = 0x10,
	FDC_ST1_NO_DATA = 0x4,
	FDC_ST1_NOT_WRITEABLE = 0x2,
	FDC_ST1_MA_MARK = 0x1,
};

enum FDC_ST2 // Used in st0 
{
	FDC_ST2_CONTROL_MARK = 0x40,
	FDC_ST2_DATA_ERROR_DD = 0x20,
	FDC_ST2_WRONG_CYLINDER = 0x10,
	FDC_ST2_SH_HIT = 0x8,
	FDC_ST2_SN_SATISFIED = 0x4,
	FDC_ST2_BAD_CYLINDER = 0x2,
	FDC_ST2_MD_MARK_DATA_FIELD = 0x1,
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

/*If the 8272 is in the DMA mode, no interrupts are generated during the execution phase. The
8272 generates DRQ's (DMA Requests) when each byte of data is available. The DMA
controller responds to this request with both a /DACK=0 (DMA Acknowledge) and a /RD=0
(Read Signal). When the DMA Acknowledge signal goes low (/DACK=0) then the DMA Request
is reset (DRQ=0). If a write command has been programmed then a /WR signal will appear
instead of /RD. After the execution phase has been completed (Terminal Count has occured)
then an interrupt will occur (INT=1). This signifies the beginning of the Result Phase. When the
first byte of data is read during the Result Phase, the Interrupt is automatically reset (INT=0*/
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
	uint8_t data[9];
	int data_current;
	int data_bytes;
	uint8_t msr;
	_Bool result_phase;
} FDC;
#endif