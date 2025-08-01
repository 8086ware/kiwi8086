#ifndef PS2_CONTROLLER_H
#define PS2_CONTROLLER_H

#include <stdint.h>

enum PS2_Ctrl_Status_Reg_Option
{
	PS2_OUTPUT_BUFFER = 1,
	PS2_INPUT_BUFFER = 2,
	PS2_SYSTEM_FLAG = 4,
	PS2_COMMAND_DATA = 8,
	PS2_TIME_OUT_ERROR = 64,
	PS2_PARITY_ERROR = 128,
};

enum PS2_Ctrl_Port
{
	PS2_DATA_PORT = 0x60,
	PS2_STATUS_COMMAND_REG = 0x64, // read = Status, write = Command
};

typedef struct PS2_Controller 
{
	uint8_t status_reg;
	uint8_t input_buffer;
	uint8_t output_buffer;
} PS2_Controller;

#endif
