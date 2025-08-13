#ifndef MDA_H
#define MDA_H

#define MDA_ADDRESS 0xB0000
#define MDA_RAM_SIZE 4096

enum MDA_Port
{
	MDA_MODE_CONTROL_REGISTER_PORT = 0x3B8,
	MDA_STATUS_REGISTER_PORT = 0x3BA,
};

typedef struct MDA 
{
	uint8_t mode_ctrl_reg;
	uint8_t status_reg;

	uint8_t ram[MDA_RAM_SIZE];
} MDA;

#endif
