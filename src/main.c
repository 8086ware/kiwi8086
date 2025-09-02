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

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	Sys8086* sys = init_sys(image, bios);
	
	SDL_Event event = { 0 };

	uint64_t last_calc_tick = 0;

	while (event.type != SDL_EVENT_QUIT) // X button on window
	{
		cpu_exec(sys); // fetch/decode/execute/store
		poll_keyboard(sys, event); // poll for keyboard events
		pit_cycle(sys); // decrement PIT counter/do other pit stuff (fire irq 0)
		display_render(sys); // Render CGA display

		int now_tick = SDL_GetTicks();

		if(now_tick - last_calc_tick >= 1000)
		{
			char title[64];
			sprintf(title, "kiwi8086 - IPS: %d, Halted: %d", sys->cpu.instructions, sys->cpu.halted);
			SDL_SetWindowTitle(sys->display.win, title);
			last_calc_tick = now_tick;
			sys->cpu.instructions = 0;
		}
		
		SDL_PollEvent(&event); // SDL_PollEvent to poll and not wait
	}

	SDL_DestroySurface(sys->display.surface);
	SDL_DestroyRenderer(sys->display.win_render);
	SDL_DestroyWindowSurface(sys->display.win);
	SDL_DestroyWindow(sys->display.win);
	SDL_DestroyPalette(sys->display.cga_palette_0);
	SDL_DestroyPalette(sys->display.cga_palette_1);
	SDL_DestroyPalette(sys->display.palette_1bit);
	SDL_DestroyPalette(sys->display.palette_4bit);

	SDL_Quit();

	free(sys);
}
