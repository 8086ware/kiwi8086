#include "system.h"
#include "pic.h"

// value is something to be written, using read 
uint8_t handle_pic_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	Pic* pic = NULL;

	if(port == PIC_MASTER_COMMAND_PORT || port == PIC_MASTER_DATA_PORT)
	{
		pic = &sys->pic_master;
	}

	else if(port == PIC_SLAVE_COMMAND_PORT || port == PIC_SLAVE_DATA_PORT)
	{
		pic = &sys->pic_slave;
	}

    switch(port)
	{
	case PIC_MASTER_COMMAND_PORT:
	case PIC_SLAVE_COMMAND_PORT:
	{
		if(read)
		{

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
	case PIC_SLAVE_DATA_PORT:
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
