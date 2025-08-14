#ifndef CGA_H
#define CGA_H

#define CGA_ADDRESS 0xB8000
#define CGA_RAM_SIZE 0x4000 // 16k of ram

enum MDA_Port
{
	MDA_MODE_CONTROL_REGISTER_PORT = 0x3B8,
	MDA_STATUS_REGISTER_PORT = 0x3BA,
};

typedef struct CGA 
{
	uint8_t mode_ctrl_reg;
	uint8_t color_ctrl_reg;

	uint8_t ram[CGA_RAM_SIZE];
} CGA;

#endif
