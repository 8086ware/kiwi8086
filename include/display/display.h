#ifndef DISPLAY_H
#define DISPLAY_H

#include "cga.h"
#include "crtc.h"
#include <SDL3/SDL.h>

typedef struct Display
{	
	// physical shit
	SDL_Window* win;
	SDL_Renderer* win_render;
	SDL_Surface* surface;
	SDL_Palette* cga_palette_0;
	SDL_Palette* cga_palette_1;
	SDL_Palette* palette_4bit;
	SDL_Palette* palette_1bit;

	CGA cga;
	Crt_controller crtc;
} Display;

SDL_Color index_cga_palette_0(uint8_t num, _Bool intensity);
SDL_Color index_cga_palette_1(uint8_t num, _Bool intensity);
SDL_Color index_4bit_palette(uint8_t num);

#endif
