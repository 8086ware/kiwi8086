#ifndef PS2_CONTROLLER_H
#define PS2_CONTROLLER_H

#include <stdint.h>

enum PS2_Ctrl_Status_Reg_Flags
{
	PS2_STATUS_OUTPUT_BUFFER_FLAG = 1,
	PS2_STATUS_INPUT_BUFFER_FLAG = 2,
	PS2_STATUS_SYSTEM_FLAG = 4,
	PS2_STATUS_COMMAND_DATA_FLAG = 8,
	PS2_STATUS_TIME_OUT_ERROR_FLAG = 64,
	PS2_STATUS_PARITY_ERROR_FLAG = 128,
};

enum PS2_Ctrl_Port
{
	PS2_STATUS_COMMAND_REG_PORT = 0x64, // read = Status, write = Command
};

typedef struct PS2_Controller 
{
	uint8_t status_reg;
	uint8_t input_buffer;
	uint8_t output_buffer;
} PS2_Controller;

#endif
