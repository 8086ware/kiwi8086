#include "system.h"
#include <SDL3/SDL.h>
#include "memory.h"
#include <stdlib.h>
#include "keyboard.h"

#define EMU_TICK_NS_PER_CYCLE (1000000000*(float)1/360)
#define EMU_CPU_NS_PER_CYCLE 200
#define EMU_PIT_NS_PER_CYCLE 838

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

	uint64_t start_frame_tick = 0;

	while (event.type != SDL_EVENT_QUIT) // X button on window
	{
		start_frame_tick = SDL_GetTicksNS();

		for (int i = 0; i < EMU_TICK_NS_PER_CYCLE / EMU_CPU_NS_PER_CYCLE; i++)
		{
			cpu_cycle(sys); // fetch/decode/execute/store
		}

		for (float i = 0; i < EMU_TICK_NS_PER_CYCLE / EMU_PIT_NS_PER_CYCLE; i++) // constant (pit)
		{
			pit_cycle(sys);
		}
		
		SDL_PollEvent(&event); // SDL_PollEvent to poll and not wait
		poll_keyboard(sys, event); // poll for keyboard events		
		display_render(sys); // Render CGA display

		while (SDL_GetTicksNS() - start_frame_tick <= EMU_TICK_NS_PER_CYCLE); // sync emulator and real time (frame rate)
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
