#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

Sys8086* init_sys(FILE* image, FILE* bios_rom)
{
	if(bios_rom == NULL || image == NULL)
	{
		return NULL;
	}

	Sys8086* sys = malloc(sizeof(Sys8086));

	if (sys == NULL)
	{
		return NULL;
	}

	memset(sys, 0, sizeof(Sys8086));

	fseek(bios_rom, 0, SEEK_END);
	int bios_size = ftell(bios_rom);
	fseek(bios_rom, 0, SEEK_SET);

	fread(&sys->memory[MAX_MEMORY_8086 - bios_size], sizeof(uint8_t), bios_size, bios_rom);

		cpu_reset(&sys->cpu);

	// init display and palettes

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

	sys->cpu.halted = 0;

	sys->fdc.fdd[0].floppy1 = image;

	return sys;
}
