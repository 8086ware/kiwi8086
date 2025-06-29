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

	for (int i = 0; i < 80 * 25; i++)
	{
		sys->display.mem[i].disp = ' ';
		sys->display.mem[i].attrib = 0;
	}

	fread(&sys->memory[0x7c00], sizeof(uint8_t), 512, image);


	return sys;
}