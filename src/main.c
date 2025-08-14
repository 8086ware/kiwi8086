#include "system.h"
#include <SDL3/SDL.h>
#include "memory.h"
#include <stdlib.h>
#include "ps2/keyboard.h"

int main(int argc, char** argv) {
	if (argc < 2)
	{
		printf("Usage: %s [IMAGE] [BIOS]\n", argv[0]);
		return 1;
	}

	FILE* image = fopen(argv[1], "r+b");

	if (image == NULL)
	{
		fprintf(stderr, "Invalid image %s.", argv[1]);
		return 2;
	}

	FILE* bios = fopen(argv[2], "r+b");

	printf("\x1B[?1049h\x1B[2J"); // enter private console screen

	Sys8086* sys = init_sys(image, bios);

	sys->cpu.ss.whole = 0x500;
	sys->cpu.sp.whole = 0xffff;
	
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	SDL_Renderer* win_render = NULL;
	SDL_Window* win;
	SDL_CreateWindowAndRenderer("kiwi8086", 80 * 9, 25 * 14, 0, &win, &win_render);

	SDL_Event event = { 0 };	

	while (event.type != SDL_EVENT_QUIT) // X button on window
	{
		printf("\x1b[H[AX=%x, BX=%x, CX=%x, DX=%x, CS=%x, DS=%x, ES=%x, SS=%x, IP=%x, SP=%x, FLAGS=%x]\n", sys->cpu.ax.whole, sys->cpu.bx.whole, sys->cpu.cx.whole, sys->cpu.dx.whole, sys->cpu.cs.whole, sys->cpu.ds.whole, sys->cpu.es.whole, sys->cpu.ss.whole, sys->cpu.ip.whole, sys->cpu.sp.whole, sys->cpu.flag.whole);
		
		printf("[Memory around CS IP: ");
		
		const int max_mem_print = 11;

		for(int i = -(max_mem_print / 2); i < max_mem_print / 2; i++)
		{
			if(i == 0)
			{
				printf("[%x] ", sys->memory[sys->cpu.cs.whole * 0x10 + (sys->cpu.ip.whole + i)]);
			}

			else 
			{
				printf("%x ", sys->memory[sys->cpu.cs.whole * 0x10 + (sys->cpu.ip.whole + i)]);
			}
		}

		cpu_exec(sys);
		ps2_keyboard(sys, event);

		SDL_SetRenderDrawColor(win_render, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(win_render);
		SDL_SetRenderDrawColor(win_render, 255, 255, 255, SDL_ALPHA_OPAQUE);

		for(int y = 0; y < 25; y++)
		{
			for(int x = 0; x < 80; x++)
			{
				uint8_t character = sys->display.ram[(y * 80 + x) * 2];
				int write_location_y = 14 * y;
				int write_location_x = 9 * x;
				int font_location = 14 * character;
				// bytes
				for(int font_y = 0; font_y < 14; font_y++)
				{
					//bits
					for(int font_x = 0; font_x < 8; font_x++)
					{
						int pixel_bit = font[font_location + font_y] << font_x;

						if(pixel_bit & 0b10000000)
						{
							SDL_RenderPoint(win_render, write_location_x + font_x, write_location_y + font_y);
						}
					}
				}
			}
		}

		SDL_PollEvent(&event); // SDL_PollEvent to poll and not wait
		SDL_RenderPresent(win_render);
	}

	free(sys->memory);
	free(sys);
	SDL_DestroyWindow(win); // frees window (including surface itself)

	SDL_Quit(); // deallocates sdl memory
	printf("\x1B[?1049l");// exit private console screen

}
