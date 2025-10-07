#include "system.h"
#include "ppi.h"

// value is something to be written, using read 
uint8_t handle_ppi_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	switch (port)
	{
	case PPI_PORT_A: // input on IBM XT
	{
		if (read)
		{
			char ret = sys->ppi.regs[0];

			if ((sys->ppi.regs[1] & PPI_FLAG_B_KEYBOARD_DISABLE))
			{
				ret = 0;
			}

			sys->ppi.regs[0] = 0;

			return ret;
		}

		break;
	}
	case PPI_PORT_B:
	{
		if (read) // output/write port
		{
			return sys->ppi.regs[1];
		}

		else
		{
			sys->ppi.regs[1] = value;

			sys->ppi.regs[2] &= ~PPI_FLAG_C_DIP_SWITCH_1;

			if (sys->ppi.regs[1] & PPI_FLAG_B_READ_SWITCHES)
			{
				sys->ppi.regs[2] |= (PPI_DIP_SWITCH  >> 4);
			}

			else
			{
				sys->ppi.regs[2] |= (PPI_DIP_SWITCH & 0x0f);
			}

			if ((sys->ppi.regs[1] & PPI_FLAG_B_KEYBOARD_CLOCK) == 0) // clock line low, respond with test pass
			{
				sys->ppi.regs[0] = 0xAA;
				sys->pic.irr |= 1 << PIC_IRQ_KEYBOARD;
			}
		}

		break;
	}
	case PPI_PORT_C:
	{
		if (read)
		{
			return sys->ppi.regs[2];
		}

		break;
	}
	case PPI_PORT_CTRL:
	{
		if (!read)
		{
			sys->ppi.control_byte = value;
		}

		break;
	}
	}
}