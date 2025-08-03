#include "system.h"
#include "pic.h"

// value is something to be written, using read 
uint8_t handle_pic_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
    switch(port)
	{
	case PIC_MASTER_COMMAND_PORT:
	{
		if(read)
		{
            //To read the ISR or IRR, write the appropriate command to the command port,
            // and then read the command port (not the data port). To read the IRR, write 0x0a. To read the ISR, write 0x0b. 
		}

		else
		{
			switch(value)
			{
			
			}
		}
		
		break;
	}

	case PIC_MASTER_DATA_PORT:
	{
		if(read)
		{
			if(sys->pic_master.command == 0)
			{
				return sys->pic_master.imr;
			}
		}
		break;
	}
	}
}
