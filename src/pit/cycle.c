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

	uint64_t times_to_execute = (now_tick - sys->pit.last_tick) / PIT_NANOSECONDS_PER_CYCLE;

	for (int i = 0; i < times_to_execute; i++)
	{
		for(int channel = 0; channel < 3; channel++)
		{
			switch(sys->pit.timers[channel].operating_mode)
			{
			case PIT_OPERATING_INTERRUPT_ON_TERMINAL_COUNT:
			{
				if(sys->pit.timers[channel].current_count != 0 && sys->pit.timers[channel].output)
				{
					sys->pit.timers[channel].current_count--;
					sys->pit.timers[channel].output = 1;
				}

				else
				{
					sys->pit.timers[channel].output = 0;
				}

				break;
			}
			case PIT_OPERATING_RATE_GENERATOR:
			{
				if(sys->pit.timers[channel].current_count != 1)
				{
					sys->pit.timers[channel].current_count--;
					sys->pit.timers[channel].output = 1;
				}

				else
				{
					sys->pit.timers[channel].output = 0;
				}

				break;
			}
			case PIT_OPERATING_SQUARE_WAVE_GENERATOR:
			{
				if(sys->pit.timers[channel].current_count % 2 == 1)
				{
					sys->pit.timers[channel].current_count -= 1;
				}

				else
				{
					sys->pit.timers[channel].current_count -= 2;
				}

				
				if(sys->pit.timers[channel].current_count != 0)
				{
					sys->pit.timers[channel].output = 1;
				}

				else
				{
					if(sys->pit.timers[channel].flip_flop == 1)
					{
						sys->pit.timers[channel].flip_flop = 0;
					}

					else
					{
						sys->pit.timers[channel].output = 0;
						sys->pit.timers[channel].flip_flop = 1;
					}
				}

				break;
			}
			}

			if(!sys->pit.timers[channel].output)
			{
				switch(sys->pit.timers[channel].operating_mode)
				{
				case PIT_OPERATING_SQUARE_WAVE_GENERATOR:
				case PIT_OPERATING_RATE_GENERATOR:
				case PIT_OPERATING_INTERRUPT_ON_TERMINAL_COUNT:
				{
					sys->pit.timers[channel].current_count = sys->pit.timers[channel].reload_value;
					break;
				}
				}

				if(channel == 0)
				{
					sys->pic.irr |= 1 << PIC_IRQ_PIT;
				}
			}
		}
	}

	sys->pit.last_tick = now_tick;
}
