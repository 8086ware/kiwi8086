#include "system.h"
#include "pic.h"

// value is something to be written, using read 
uint8_t handle_pic_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	Pic* pic = NULL;

	if (port == PIC_MASTER_COMMAND_PORT || port == PIC_MASTER_DATA_PORT)
	{
		pic = &sys->pic_master;
	}

	else if (port == PIC_SLAVE_COMMAND_PORT || port == PIC_SLAVE_DATA_PORT)
	{
		pic = &sys->pic_slave;
	}

	switch (port)
	{
	case PIC_MASTER_COMMAND_PORT:
	case PIC_SLAVE_COMMAND_PORT:
	{
		if (read)
		{

		}

		else
		{
			if (value & PIC_ICW1_INIT)
			{
				pic->icw[0] = value;
				pic->icw_step = 2;
				pic->imr = 0;
			}

			else
			{
				if (value & PIC_OCW3_ENABLE) // OCW3 selected
				{
					pic->ocw[2] = value;
				}

				else // OCW2 selected
				{
					pic->ocw[1] = value;
					uint8_t ir_acted_upon = pic->ocw[1] & PIC_OCW2_SERVICING;
				
					if (pic->ocw[1] & PIC_OCW2_EOI)
					{
						for (int i = 0; i < 8; i++)
						{
							if (pic->isr & (1 << i))
							{
								pic->isr &= ~(1 << i);
								break;
							}
						}
					}
				}
			}
		}

		break;
	}

	case PIC_MASTER_DATA_PORT:
	case PIC_SLAVE_DATA_PORT:
	{
		if (read)
		{
			if (pic->command == 0) // OCW1
			{
				return pic->imr;
			}
		}

		else
		{
			if (pic->command == 0 && pic->icw_step == 0) // OCW1 
			{
				pic->imr = value;
			}

			if (pic->icw_step == 2)
			{
				pic->vector_offset = value & 0xF8; // in 8086 mode the 5 most significant data bits are the vector offset
				pic->icw_step = 0;

				if ((pic->icw[0] & PIC_ICW1_SINGLE) == 0)
				{
					pic->icw_step = 3;
				}

				else if (pic->icw[0] & PIC_ICW1_ICW4)
				{
					pic->icw_step = 4;
				}

				else
				{
					pic->icw[3] = 0;
				}
			}

			else if (pic->icw_step == 3) // icw3 only read when 2 pics
			{
				pic->icw[2] = value;

				if (pic->icw[0] & PIC_ICW1_ICW4)
				{
					pic->icw_step = 4;
				}

				else
				{
					pic->icw[3] = 0;
					pic->icw_step = 0;
				}
			}

			else if (pic->icw_step == 4)
			{
				pic->icw[3] = value;
				pic->icw_step = 0;
			}
		}
		break;
	}
	}
}