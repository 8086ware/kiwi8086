#include "system.h"
#include "pit.h"
#include <SDL3/SDL.h>

// pit logic
// decrease 1 each 838 ns
// when current counter is zero reload value is loaded again
// restart

void pit_cycle(Sys8086* sys)
{
	uint64_t now_tick = SDL_GetTicksNS();

	if (now_tick - sys->pit.last_tick >= PIT_NANOSECONDS_PER_CYCLE)
	{
		for (int i = 0; i < 3; i++)
		{
			int64_t real_current_count = sys->pit.timers[i].current_count;
			uint16_t former_count = sys->pit.timers[i].current_count;

			sys->pit.timers[i].current_count -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;

			real_current_count -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;

			if (real_current_count <= 0 && former_count != 0)
			{
				sys->pit.timers[i].current_count = sys->pit.timers[i].reload_value;

				switch (i)
				{
				case 0:
				{
					sys->pic_master.irr |= (1 << PIC_IRQ_PIT);
					break;
				}
				case 1:
				{
					break;
				}
				}
			}

			sys->pit.last_tick = now_tick;
		}
	}
}