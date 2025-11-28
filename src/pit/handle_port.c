#include "system.h"
#include "pit.h"

// value is something to be written, using read 
uint8_t handle_pit_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	switch (port)
	{
	case PIT_CHANNEL_0_PORT:
	case PIT_CHANNEL_1_PORT:
	case PIT_CHANNEL_2_PORT:
	{
		int channel = port - PIT_CHANNEL_0_PORT;

		if (read)
		{
			if (!sys->pit.timers[channel].sent_current_low) // low
			{
				sys->pit.timers[channel].sent_current_low = 1;

				if (sys->pit.timers[channel].access_mode != PIT_ACCESS_HIBYTE_ONLY) // Lobyte or both?
				{
					if (sys->pit.timers[channel].latch_on)
					{
						sys->pit.timers[channel].latch_on = 0;
						return sys->pit.timers[channel].latch & 0x00ff;
					}

					else
					{
						return sys->pit.timers[channel].current_count & 0x00ff;
					}
				}

				else
				{
					return 0;
				}
			}

			else // high
			{
				sys->pit.timers[channel].sent_current_low = 0;

				if (sys->pit.timers[channel].access_mode != PIT_ACCESS_LOBYTE_ONLY)  // Hibyte or both?
				{
					if (sys->pit.timers[channel].latch_on)
					{
						sys->pit.timers[channel].latch_on = 0;
						return sys->pit.timers[channel].latch >> 8;
					}

					else
					{
						return sys->pit.timers[channel].current_count >> 8;
					}
				}

				else
				{
					return 0;
				}
			}
		}

		else
		{
			if (!sys->pit.timers[channel].received_reload_low) // low
			{
				sys->pit.timers[channel].received_reload_low = 1;

				if (sys->pit.timers[channel].access_mode != PIT_ACCESS_HIBYTE_ONLY)
				{
					sys->pit.timers[channel].reload_value &= ~0x00ff;
					sys->pit.timers[channel].reload_value |= value;

				}

				else if (sys->pit.timers[channel].access_mode == PIT_ACCESS_LOBYTE_ONLY)
				{
					if (sys->pit.timers[channel].operating_mode == PIT_OPERATING_INTERRUPT_ON_TERMINAL_COUNT)
					{
						sys->pit.timers[channel].output = 1;
						sys->pit.timers[channel].current_count = sys->pit.timers[channel].reload_value;
					}
				}
			}

			else // high
			{
				sys->pit.timers[channel].received_reload_low = 0;

				if (sys->pit.timers[channel].access_mode != PIT_ACCESS_LOBYTE_ONLY)
				{
					sys->pit.timers[channel].reload_value &= ~0xff00;
					sys->pit.timers[channel].reload_value |= (value << 8);

					if (sys->pit.timers[channel].operating_mode == PIT_OPERATING_INTERRUPT_ON_TERMINAL_COUNT)
					{
						sys->pit.timers[channel].output = 1;
						sys->pit.timers[channel].current_count = sys->pit.timers[channel].reload_value;
					}
				}
			}
		}

		break;
	}

	case PIT_MODE_COMMAND_REG_PORT:
	{
		if (read)
		{

		}

		else
		{
			int channel = (value & 0xC0) >> 6;

			if (((value & 0x30) >> 4) == 0 && sys->pit.timers[channel].access_mode == PIT_ACCESS_LOBYTE_HIBYTE)
			{
				sys->pit.timers[channel].latch_on = 1;
				sys->pit.timers[channel].latch = sys->pit.timers[channel].current_count;
				sys->pit.timers[channel].sent_current_low = 0;
			}

			else
			{
				sys->pit.timers[channel].access_mode = (value & 0x30) >> 4;
				sys->pit.timers[channel].operating_mode = (value & 0xE) >> 1;

				if (sys->pit.timers[channel].operating_mode == PIT_OPERATING_INTERRUPT_ON_TERMINAL_COUNT)
				{
					sys->pit.timers[channel].output = 0;
				}
			}
		}
		break;
	}
	}
}
