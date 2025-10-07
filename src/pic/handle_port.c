#include "system.h"
#include "pic.h"

// value is something to be written, using read 
uint8_t handle_pic_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	switch (port)
	{
	case PIC_MASTER_COMMAND_PORT:
	{
		if (read)
		{
			if((sys->pic.ocw[2] & PIC_OCW3_READ_REGISTER) == 0x2) // Read IR reg
			{
				return sys->pic.irr;
			}

			else if((sys->pic.ocw[2] & PIC_OCW3_READ_REGISTER) == 0x3) // Read IS reg
			{
				return sys->pic.isr;
		}

			sys->pic.ocw[2] &= ~PIC_OCW3_READ_REGISTER;
		}

		else
		{
			if (value & PIC_ICW1_INIT)
			{
				sys->pic.icw[0] = value;
				sys->pic.icw_step = 2;
				sys->pic.imr = 0;
			}

			else
			{
				if (value & PIC_OCW3_ENABLE) // OCW3 selected
				{
					sys->pic.ocw[2] = value;
				}

				else // OCW2 selected
				{
					sys->pic.ocw[1] = value;
				
					uint8_t ir_acted_upon = sys->pic.ocw[1] & PIC_OCW2_SERVICING;
				
					if (sys->pic.ocw[1] & PIC_OCW2_EOI)
					{
						if(sys->pic.ocw[1] & PIC_OCW2_SL)
						{
							sys->pic.isr &= ~(1 << ir_acted_upon);
						}

						for (int i = 0; i < 8; i++)
						{
							if (sys->pic.isr & (1 << i))
							{
								sys->pic.isr &= ~(1 << i);
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
	{
		if (read)
		{
			if (sys->pic.command == 0) // OCW1
			{
				return sys->pic.imr;
			}
		}

		else
		{
			if (sys->pic.command == 0 && sys->pic.icw_step == 0) // OCW1 
			{
				sys->pic.imr = value;
			}

			if (sys->pic.icw_step == 2)
			{
				sys->pic.vector_offset = value & 0xF8; // in 8086 mode the 5 most significant data bits are the vector offset
				sys->pic.icw_step = 0;

				if ((sys->pic.icw[0] & PIC_ICW1_SINGLE) == 0)
				{
					sys->pic.icw_step = 3;
				}

				else if (sys->pic.icw[0] & PIC_ICW1_ICW4)
				{
					sys->pic.icw_step = 4;
				}

				else
				{
					sys->pic.icw[3] = 0;
				}
			}

			else if (sys->pic.icw_step == 3) // icw3 only read when 2 pics
			{
				sys->pic.icw[2] = value;

				if (sys->pic.icw[0] & PIC_ICW1_ICW4)
				{
					sys->pic.icw_step = 4;
				}

				else
				{
					sys->pic.icw[3] = 0;
					sys->pic.icw_step = 0;
				}
			}

			else if (sys->pic.icw_step == 4)
			{
				sys->pic.icw[3] = value;
				sys->pic.icw_step = 0;
			}
		}
		break;
	}
	}
}