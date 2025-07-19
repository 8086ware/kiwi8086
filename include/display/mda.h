#ifndef CGA_H
#define CGA_H

#include <stdint.h>

#define MDA_RAM_SIZE 4096

enum MDA_Port
{
	MDA_MODE_CONTROL_REGISTER = 0x3B8,
	MDA_STATUS_REGISTER = 0x3BA,
};

typedef struct MDA 
{
	uint8_t mode_ctrl_reg;
	uint8_t status_reg;

	uint8_t ram[MDA_RAM_SIZE];

	uint16_t cursor_address;
} MDA;

#endif
