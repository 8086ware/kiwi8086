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
        if (read)
        {
            switch (sys->display.crtc.index)
            {
            case CRTC_DATA_CURSOR_START_SCANLINE:
            {
                return sys->display.crtc.cursor_start_scan_line;
                break;
            }
            case CRTC_DATA_CURSOR_END_SCANLINE:
            {
                return sys->display.crtc.cursor_end_scan_line;
                break;
            }
            case CRTC_DATA_START_ADDRESS_HIGH:
            {
                return (sys->display.crtc.start_address & 0xff00) >> 8;
                break;
            }
            case CRTC_DATA_START_ADDRESS_LOW:
            {
                return sys->display.crtc.start_address & 0x00ff;
                break;
            }
            case CRTC_DATA_CURSOR_ADDRESS_HIGH:
            {
                return (sys->display.crtc.cursor_address & 0xff00) >> 8;
                break;
            }
            case CRTC_DATA_CURSOR_ADDRESS_LOW:
            {
                return sys->display.crtc.cursor_address & 0x00ff;
                break;
            }
            }
        }
        else
        {
            switch (sys->display.crtc.index)
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
                uint16_t real_value = value;
                real_value <<= 8;
                real_value |= (sys->display.crtc.start_address & 0x00ff);
                sys->display.crtc.start_address = real_value;
                break;
            }
            case CRTC_DATA_START_ADDRESS_LOW:
            {
                uint16_t real_value = value;
                real_value |= (sys->display.crtc.start_address & 0xff00);
                sys->display.crtc.start_address = real_value;
                break;
            }
            case CRTC_DATA_CURSOR_ADDRESS_HIGH:
            {
                uint16_t real_value = value;
                real_value <<= 8;
                real_value |= (sys->display.crtc.cursor_address & 0x00ff);
                sys->display.crtc.cursor_address = real_value;
                break;
            }
            case CRTC_DATA_CURSOR_ADDRESS_LOW:
            {
                uint16_t real_value = value;
                real_value |= (sys->display.crtc.cursor_address & 0xff00);
                sys->display.crtc.cursor_address = real_value;
                break;
            }
        }
        }
        break;
    }
	}
}
