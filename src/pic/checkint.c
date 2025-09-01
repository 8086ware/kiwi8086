#include "system.h"
#include "cpu/instructions.h"
#include "memory.h"

void pic_check_int(Sys8086* sys)
{
	_Bool in_service = 0;
	_Bool slave = 0;

	uint16_t vector_offset = 0;

	if (sys->cpu.flag.whole & FLAG_INTERRUPT)
	{
		if (sys->pic_master.irr != 0 || sys->pic_slave.irr != 0)
		{
			for (int i = 0; i < 8; i++)
			{
				if (sys->pic_master.irr & (1 << i))
				{
					if (sys->pic_master.isr & (1 << i))
					{
						in_service = 1;
						break;
					}

					else if ((sys->pic_master.icw[0] & PIC_ICW1_SINGLE) == 0) // Check the slave PIC if available
					{
						if ((sys->pic_master.irr & (1 << i)) == sys->pic_master.icw[2])
						{
							for (int i = 0; i < 8; i++)
							{
								if (sys->pic_slave.isr & (1 << i))
								{
									in_service = 1;
									break;
								}

								else if (sys->pic_slave.irr & (1 << i))
								{
									sys->pic_slave.isr |= (1 << i);
									sys->pic_slave.irr &= ~(1 << i);

									vector_offset = (sys->pic_master.vector_offset * 4) + i * 4;
									slave = 1;
									break;
								}
							}
						}
					}

					if (!slave)
					{
						vector_offset = (sys->pic_master.vector_offset * 4) + i * 4;
					}

					sys->pic_master.isr |= (1 << i);
					sys->pic_master.irr &= ~(1 << i);
					break;
				}
			}

			if (!in_service)
			{
				push(sys, sys->cpu.flag.whole);
				push(sys, sys->cpu.cs.whole);
				push(sys, sys->cpu.ip.whole);

				jmp(sys, read_address16(sys, seg_mem(0, vector_offset + 2), 0), read_address16(sys, seg_mem(0, vector_offset), 0));
				
				sys->cpu.halted = 0;
				sys->cpu.flag.whole &= ~FLAG_INTERRUPT;
			}
		}
	}
}