#include "system.h"
#include <stdio.h>
#include <stdlib.h>

Sys8086* init_sys(FILE* image)
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

	sys->cpu.cs.whole = 0x0;
	sys->cpu.ip.whole = 0x7c00;
	
	sys->display.columns = 80;
	sys->display.rows = 25;

	sys->display.mem = &sys->memory[0xB8000];


	for (int i = 0; i < VGA_DISPLAY_PAGES; i++)
	{
		sys->display.cursor_x[i] = 0;
		sys->display.cursor_y[i] = 0;
	}

	sys->display.display_page = 0;

	for (int i = 0; i < MAX_MEMORY_8086; i++)
	{
		sys->memory[i] = 0;
	}

	sys->cpu.flag.whole = 0;

	sys->pic_master.vector_offset = 0x08;
	sys->pic_slave.vector_offset = 0x70;

	sys->pic_master.irq = -1;
	sys->pic_slave.irq = -1;

	sys->pic_master.imr = 0;
	sys->pic_slave.imr = 0;

	sys->cpu.halted = 0;

	fread(&sys->memory[0x7c00], sizeof(uint8_t), 512, image);
	return sys;
}