#include "memory.h"
#include "display/crtc.h"
#include "system.h"

// value is something to be written, using read 
uint8_t handle_crtc_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
    switch(port)
	{
	case CRTC_INDEX_REGISTER_PORT:
	{
        if(read)
        {
            return 0;
        }

        else
        {
            sys->display.crtc.index = value;
        }
		break;
	}
    case CRTC_DATA_REGISTER_PORT:
    {
        switch(sys->display.crtc.index)
        {
            case CRTC_DATA_CURSOR_START_SCANLINE:
            {
                sys->display.crtc.cursor_start_scan_line = value;
                break;
            }
            case CRTC_DATA_CURSOR_END_SCANLINE:
            {
                sys->display.crtc.cursor_end_scan_line = value;
                break;
            }
            case CRTC_DATA_START_ADDRESS_HIGH:
            {
                sys->display.crtc.start_address |= (((uint16_t)value) << 8);
                break;
            }
            case CRTC_DATA_START_ADDRESS_LOW:
            {
                sys->display.crtc.start_address |= value;
                break;
            }
            case CRTC_DATA_CURSOR_ADDRESS_HIGH:
            {
                sys->display.crtc.cursor_address |= (((uint16_t)value) << 8);
                break;
            }
            case CRTC_DATA_CURSOR_ADDRESS_LOW:
            {
                sys->display.crtc.cursor_address |= (uint16_t)value;
                break;
            }
        }
        break;
    }
	}
}
