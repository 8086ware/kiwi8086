#include "system.h"
#include <stdio.h>
#include <stdlib.h>

Sys8086* init_sys(FILE* image, FILE* bios_rom)
{
	Sys8086* sys = malloc(sizeof(Sys8086));

	if (sys == NULL)
	{
		return NULL;
	}

	sys->memory = malloc(sizeof(uint8_t) * MAX_MEMORY_8086);

	if (sys->memory == NULL)
	{
		return NULL;
	}

	if(bios_rom == NULL)
	{
		// Starting point of first bootsector
		sys->cpu.cs.whole = 0x0;
		sys->cpu.ip.whole = 0x7c00;
	}

	else
	{
		// Starting point of a normal reset 8086
		sys->cpu.cs.whole = 0xFFFF;
		sys->cpu.ip.whole = 0x0;
	}

	// init mda
	
	sys->display.mode_ctrl_reg = 0b00011001;
	sys->display.status_reg = 0b00001001;

	// init crtc

	sys->crtc.cursor_address = 0;

	for (int i = 0; i < MDA_RAM_SIZE; i++)
	{
		sys->display.ram[i] = 0;
	}

	// init memory
	for (int i = 0; i < MAX_MEMORY_8086; i++)
	{
		sys->memory[i] = 0;
	}

	// init pic
	sys->pic_master.vector_offset = 0x20;
	sys->pic_slave.vector_offset = 0x1C0;

	sys->pic_master.irr = 0;
	sys->pic_slave.irr = 0;

	sys->pic_master.imr = 0;
	sys->pic_slave.imr = 0;

	sys->pic_master.isr = 0;
	sys->pic_slave.isr = 0;

	// init ps2
	
	sys->ps2.status_reg = 0;

	sys->cpu.halted = 0;

	sys->cpu.prev_byte_success = 0;

	sys->cpu.flag.whole = 0;

	// load bios if exist and load bootloader raw if not
	if(bios_rom != NULL)
	{
		fseek(bios_rom, 0, SEEK_END);
		int bios_size = ftell(bios_rom);
		fseek(bios_rom, 0, SEEK_SET);

		fread(&sys->memory[MAX_MEMORY_8086 - bios_size], sizeof(uint8_t), bios_size, bios_rom);
	}

	else 
	{
		fread(&sys->memory[0x7c00], sizeof(uint8_t), 512, image);
	}

	return sys;
}
