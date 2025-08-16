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
            if(!sys->pit.timers[port - PIT_CHANNEL_0_PORT].sent_current_low)
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].sent_current_low = 1;
                return sys->pit.timers[port - PIT_CHANNEL_0_PORT].current_count & 0x00ff;
            }

            else
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].sent_current_low = 0;
                return (sys->pit.timers[port - PIT_CHANNEL_0_PORT].current_count & 0xff000) >> 8;
            }
        }

		else
        {
            if(!sys->pit.timers[port - PIT_CHANNEL_0_PORT].received_reload_low)
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].received_reload_low = 1;
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].reload_value = value;
            }

            else
            {
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].received_reload_low = 0;
                uint16_t upper_byte = value << 8;
                sys->pit.timers[port - PIT_CHANNEL_0_PORT].reload_value |= upper_byte;
            }
		}
		
		break;
	}
	}
}
