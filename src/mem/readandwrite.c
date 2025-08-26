#include "memory.h"
#include <stdint.h>
#include "system.h"
#include "pic.h"
#include <stdio.h>
#include "ps2/controller.h"
#include "display/cga.h"

void write_address8(Sys8086* sys, uint32_t address, uint8_t value, _Bool port)
{
	if (address >= MAX_MEMORY_8086)
	{
		return;
	}

	if (port) // port address space (devices)
	{
		switch(address)
		{
		case 0x80:
		{
			printf("POST CODE: %d\n", value);
			break;
		}
		case 0xE9:
		{
			printf("-------------PORT 0xE9 HACK:%c\n", value);
			break;
		}
		case PS2_STATUS_COMMAND_REG_PORT:
		{
			handle_ps2_controller_port(sys, address, value, 0);
			break;
		}
		case PIC_MASTER_COMMAND_PORT:
		case PIC_SLAVE_COMMAND_PORT:
		case PIC_MASTER_DATA_PORT:
		case PIC_SLAVE_DATA_PORT:
		{
			handle_pic_port(sys, address, value, 0);
			break;
		}
		case CGA_MODE_CONTROL_REGISTER_PORT:
		case CGA_COLOR_CONTROL_REGISTER_PORT:
		case CGA_STATUS_REGISTER_PORT:
		{
			handle_cga_port(sys, address, value, 0);
			break;
		}
		case CRTC_INDEX_REGISTER_PORT:
		case CRTC_DATA_REGISTER_PORT:
		{
			handle_crtc_port(sys, address, value, 0);
			break;
		}
		case PIT_CHANNEL_0_PORT:
		case PIT_CHANNEL_1_PORT:
		case PIT_CHANNEL_2_PORT:
		case PIT_MODE_COMMAND_REG_PORT:
		{
			handle_pit_port(sys, address, value, 0);
			break;
		}
		}
	}

	else // normal address space
	{
		if(address >= CGA_ADDRESS && address <= CGA_ADDRESS + CGA_RAM_SIZE)
		{
			sys->display.cga.ram[address - CGA_ADDRESS] = value;
		}

		switch (address)
		{
		default:
		{
			sys->memory[address] = value;
		}
		}
	}
}

void write_address16(Sys8086* sys, uint32_t address, uint16_t value, _Bool port)
{
	write_address8(sys, address, value & 0x00ff, port);
	write_address8(sys, address + 1, (value & 0xff00) >> 8, port);
}

uint8_t read_address8(Sys8086* sys, uint32_t address, _Bool port)
{
	if (address >= MAX_MEMORY_8086)
	{
		return -1;
	}

	if (port) // port address space (devices)
	{
		switch (address)
		{
		case PS2_STATUS_COMMAND_REG_PORT:
		{
			return handle_ps2_controller_port(sys, address, 0, 1);
			break;
		}
		case PIC_MASTER_COMMAND_PORT:
		case PIC_SLAVE_COMMAND_PORT:
		case PIC_MASTER_DATA_PORT:
		case PIC_SLAVE_DATA_PORT:
		{
			return handle_pic_port(sys, address, 0, 1);
			break;
		}
		case CGA_MODE_CONTROL_REGISTER_PORT:
		case CGA_COLOR_CONTROL_REGISTER_PORT:
		case CGA_STATUS_REGISTER_PORT:
		{
			return handle_cga_port(sys, address, 0, 1);
			break;
		}
		case CRTC_INDEX_REGISTER_PORT:
		case CRTC_DATA_REGISTER_PORT:
		{
			return handle_crtc_port(sys, address, 0, 1);
		}
		case PIT_CHANNEL_0_PORT:
		case PIT_CHANNEL_1_PORT:
		case PIT_CHANNEL_2_PORT:
		case PIT_MODE_COMMAND_REG_PORT:
		{
			return handle_pit_port(sys, address, 0, 1);
			break;
		}
		}
	}

	else // normal address space
	{
		if(address >= CGA_ADDRESS && address <= CGA_ADDRESS + CGA_RAM_SIZE)
		{
			return sys->display.cga.ram[address - CGA_ADDRESS];
		}

		switch (address)
		{
		default:
		{
			return sys->memory[address];
			break;
		}
		}
	}

	return 0;
}

uint16_t read_address16(Sys8086* sys, uint32_t address, _Bool port)
{
	uint16_t value = read_address8(sys, address, port);
	value |= (uint16_t)read_address8(sys, address + 1, port) << 8;
	return value;
}
