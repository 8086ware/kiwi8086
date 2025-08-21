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

	if(now_tick - sys->pit.last_tick >= PIT_NANOSECONDS_PER_CYCLE)
	{
		int64_t real_current_count0 = sys->pit.timers[0].current_count;
		int64_t real_current_count1 = sys->pit.timers[1].current_count;
		int64_t real_current_count2 = sys->pit.timers[2].current_count;

		uint16_t former_count0 = sys->pit.timers[0].current_count;
		uint16_t former_count1 = sys->pit.timers[1].current_count;
		uint16_t former_count2 = sys->pit.timers[2].current_count;

		sys->pit.timers[0].current_count -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;
		sys->pit.timers[1].current_count -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;
		sys->pit.timers[2].current_count -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;

		real_current_count0 -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;
		real_current_count1 -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;
		real_current_count2 -= (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;

		if (real_current_count0 <= 0 && former_count0 != 0)
		{
			sys->pit.timers[0].current_count = sys->pit.timers[0].reload_value;
			sys->pic_master.irr |= (1 << PIC_IRQ_PIT);
		}

		sys->pit.last_tick = now_tick;
	}
}

