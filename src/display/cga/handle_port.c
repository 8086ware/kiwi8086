#include "memory.h"
#include "display/cga.h"
#include "system.h"

// value is something to be written, using read 
uint8_t handle_cga_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
    switch(port)
	{
	case CGA_STATUS_REGISTER_PORT:
	{
		if(read)
		{
			return 0b0001001; // nevre in vertical retrace (technically) and no CGA snow
		}


		break;
	}
	case CGA_MODE_CONTROL_REGISTER_PORT:
	{
		{
			sys->display.cga.mode_ctrl_reg = value;

			if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_HIGH_RES_GRAPHICS)
			{
				SDL_DestroySurface(sys->display.surface);
				sys->display.surface = SDL_CreateSurface(640, 200, SDL_PIXELFORMAT_INDEX1MSB);
				SDL_SetSurfacePalette(sys->display.surface, sys->display.palette_1bit);
			}

			else if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_GRAPHICS)
			{
				SDL_DestroySurface(sys->display.surface);
				sys->display.surface = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_INDEX2MSB);
				if(sys->display.cga.color_ctrl_reg & CGA_COLOR_CONTROL_PALETTE) // magenta, cyan, white
				{
					SDL_SetSurfacePalette(sys->display.surface, sys->display.cga_palette_1);
				}

				else // red, green, yellow
				{
					SDL_SetSurfacePalette(sys->display.surface, sys->display.cga_palette_0);
				}
			}
			
			else
			{
				SDL_DestroySurface(sys->display.surface);

				if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_HIGH_RES)
				{
					sys->display.surface = SDL_CreateSurface(640, 200, SDL_PIXELFORMAT_INDEX4MSB);
				}
				else
				{
					sys->display.surface = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_INDEX4MSB);
				}

				SDL_SetSurfacePalette(sys->display.surface, sys->display.palette_4bit);
			}
		}
		
		break;
	}
	case CGA_COLOR_CONTROL_REGISTER_PORT:
	{
		if(read)
		{
			return 0;
		}

		else
		{
			sys->display.cga.color_ctrl_reg = value;

			// CGA_COLOR_CONTROL_BORDER_BACKGROUND_FOREGROUND set
			
			if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_HIGH_RES_GRAPHICS && sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_GRAPHICS) // 640x200 1bpp (foreground of bit)
			{
				sys->display.palette_1bit->colors[1] = index_4bit_palette(sys->display.cga.color_ctrl_reg & CGA_COLOR_CONTROL_BORDER_BACKGROUND_FOREGROUND);
				SDL_SetPaletteColors(sys->display.palette_1bit, sys->display.palette_1bit->colors, 0, 2);
			}
			
			else if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_GRAPHICS) // 320x200 2bpp (background)
			{
				sys->display.cga_palette_0->colors[0] = index_4bit_palette(sys->display.cga.color_ctrl_reg & CGA_COLOR_CONTROL_BORDER_BACKGROUND_FOREGROUND);
				sys->display.cga_palette_1->colors[0] = index_4bit_palette(sys->display.cga.color_ctrl_reg & CGA_COLOR_CONTROL_BORDER_BACKGROUND_FOREGROUND);

				if(sys->display.cga.color_ctrl_reg & CGA_COLOR_CONTROL_PALETTE) // magenta, cyan, white
				{
					SDL_SetSurfacePalette(sys->display.surface, sys->display.cga_palette_1);
				}

				else // red, green, yellow
				{
					SDL_SetSurfacePalette(sys->display.surface, sys->display.cga_palette_0);
				}

				SDL_SetPaletteColors(sys->display.cga_palette_0, sys->display.cga_palette_0->colors, 0, 4);
			}
		}

		break;
	}
	}
}
