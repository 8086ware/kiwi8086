#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

Sys8086* init_sys(FILE* image, FILE* bios_rom)
{
	Sys8086* sys = malloc(sizeof(Sys8086));

	if (sys == NULL)
	{
		return NULL;
	}

	memset(sys, 0, sizeof(Sys8086));

	if(bios_rom == NULL)
	{
		// Starting point of first bootsector
		sys->cpu.cs.whole = 0x0;
		sys->cpu.ip.whole = 0x7c00;
	}

	else
	{
		cpu_reset(&sys->cpu);
	}

	// init display and palettes
	// init cga
	
	sys->display.cga.color_ctrl_reg = 0;
	sys->display.cga.mode_ctrl_reg = 0;

	sys->display.surface = NULL;

	SDL_CreateWindowAndRenderer("kiwi8086", 800, 600, SDL_WINDOW_RESIZABLE, &sys->display.win, &sys->display.win_render); // physical CGA window

	SDL_Color color_4bit[16];

	for(int i = 0; i < 16; i++)
	{
		color_4bit[i] = index_4bit_palette(i);
	}

	sys->display.palette_4bit = SDL_CreatePalette(16);
	SDL_SetPaletteColors(sys->display.palette_4bit, color_4bit, 0, 16);

	SDL_Color color_1bit[2] = {{0, 0, 0, 0}, {0xFF, 0xFF, 0xFF, 0xFF}};
	sys->display.palette_1bit = SDL_CreatePalette(2);
	SDL_SetPaletteColors(sys->display.palette_1bit, color_1bit, 0, 2);

	SDL_Color color_2bit_0[4];

	for(int i = 0; i < 4; i++)
	{
		color_2bit_0[i] = index_cga_palette_0(i, 0);
	}

	sys->display.cga_palette_0 = SDL_CreatePalette(4);
	SDL_SetPaletteColors(sys->display.cga_palette_0, color_2bit_0, 0, 4);

	SDL_Color color_2bit_1[4];

	for(int i = 0; i < 4; i++)
	{
		color_2bit_1[i] = index_cga_palette_1(i, 0);
	}

	sys->display.cga_palette_1 = SDL_CreatePalette(4);
	SDL_SetPaletteColors(sys->display.cga_palette_1, color_2bit_1, 0, 4);

	memset(sys->display.cga.ram, 0, CGA_RAM_SIZE);

	// init crtc

	sys->display.crtc.start_address = 0;
	sys->display.crtc.cursor_address = 0;
	
	sys->display.crtc.cursor_start_scan_line = 6;
	sys->display.crtc.cursor_end_scan_line = 8;

	// init memory

	memset(sys->memory, 0, MAX_MEMORY_8086);

	// init pic
	sys->pic_master.vector_offset = 0x20;
	sys->pic_slave.vector_offset = 0x1C0;

	sys->pic_master.irr = 0;
	sys->pic_slave.irr = 0;

	sys->pic_master.imr = 0;
	sys->pic_slave.imr = 0;

	sys->pic_master.isr = 0;
	sys->pic_slave.isr = 0;

	sys->cpu.halted = 0;

	sys->cpu.flag.whole = 0;

	// init pit
	
	sys->pit.timers[0].reload_value = 0;
	sys->pit.timers[0].current_count = sys->pit.timers[0].reload_value;

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
