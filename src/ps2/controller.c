#include "ps2/controller.h"
#include "system.h"

// value is something to be written, using read 
uint8_t handle_ps2_controller_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	// if read, that means its from read_address, if !read, it means its from write address
	switch(port)
	{
	case PS2_STATUS_COMMAND_REG:
	{
		if(read)
		{
			return sys->ps2.status_reg;
		}

		else
		{
			switch(value)
			{
			case 0xAA:
			{
				sys->ps2.output_buffer = 0x55;
				sys->ps2.status_reg |= PS2_OUTPUT_BUFFER;
				break;
			}
			}
		}
		
		break;
	}

	case PS2_DATA_PORT:
	{
		if(read)
		{
			sys->ps2.status_reg &= ~PS2_OUTPUT_BUFFER;
			return sys->ps2.output_buffer;
		}
		break;
	}
	}
}
