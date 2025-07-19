#include "memory.h"
#include <stdint.h>
#include "system.h"

void write_address8(Sys8086* sys, uint32_t address, uint8_t value, _Bool port)
{
	if (port) // port address space (devices)
	{
		
	}

	else // normal address space (physical ram)
	{
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
	write_address8(sys, address + 1, value & 0xff00, port);
}

	if (port) // port address space (devices)
	{

	}

	else // normal address space (physical ram)
	{
		switch (address)
		{
		default:
		{
			*valueptr = value;

			break;
		}
		}
	}
}

uint8_t read_address8(Sys8086* sys, uint32_t address, _Bool port)
{
	if (port) // port address space (devices)
	{

	}

	else // normal address space (physical ram)
	{
		switch (address)
		{
		default:
		{
			return sys->memory[address];
			break;
		}
		}
	}
}

uint16_t read_address16(Sys8086* sys, uint32_t address, _Bool port)
{
	uint16_t value = read_address8(sys, address, port);
	value |= (uint16_t)read_address8(sys, address + 1, port) << 8;
}
