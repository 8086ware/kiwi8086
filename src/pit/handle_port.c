#include "system.h"
#include "pit.h"

// value is something to be written, using read 
uint8_t handle_pit_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
    switch(port)
	{
	case PIT_CHANNEL_0_PORT:
	case PIT_CHANNEL_1_PORT:
	case PIT_CHANNEL_2_PORT:
	{
		if(read)
		{
            if(!sys->pit.timers[port - PIT_CHANNEL_0_PORT].sent_current_low) // low
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].sent_current_low = 1;

                if (sys->pit.timers[port - PIT_CHANNEL_0_PORT].access_mode != PIT_ACCESS_HIBYTE_ONLY)
                {
                return sys->pit.timers[port - PIT_CHANNEL_0_PORT].current_count & 0x00ff;
            }

            else
            {
                    return 0;
                }
            }

            else // high
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].sent_current_low = 0;

                if (sys->pit.timers[port - PIT_CHANNEL_0_PORT].access_mode != PIT_ACCESS_LOBYTE_ONLY) 
                {
                    return sys->pit.timers[port - PIT_CHANNEL_0_PORT].current_count >> 8;
                }

                else
                {
                    return 0;
                }
            }
        }

		else
        {
            if(!sys->pit.timers[port - PIT_CHANNEL_0_PORT].received_reload_low) // low
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].received_reload_low = 1;

                if (sys->pit.timers[port - PIT_CHANNEL_0_PORT].access_mode != PIT_ACCESS_HIBYTE_ONLY)
                {
                    sys->pit.timers[port - PIT_CHANNEL_0_PORT].reload_value &= ~0x00ff;
                    sys->pit.timers[port - PIT_CHANNEL_0_PORT].reload_value |= value;
                }
            }

            else // high
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].received_reload_low = 0;

                if (sys->pit.timers[port - PIT_CHANNEL_0_PORT].access_mode != PIT_ACCESS_LOBYTE_ONLY)
                {
                    sys->pit.timers[port - PIT_CHANNEL_0_PORT].reload_value &= ~0xff00;
                    sys->pit.timers[port - PIT_CHANNEL_0_PORT].reload_value |= (value << 8);
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
            sys->pit.timers[channel].access_mode = (value & 0x30) >> 4;
            sys->pit.timers[channel].operating_mode = (value & 0xE) >> 1;
        }
		break;
	}
	}
}
