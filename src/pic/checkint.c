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
		if (sys->pic_master.irr != 0)
		{
			for (int i = 0; i < 8; i++)
			{
				if (sys->pic_master.irr & (1 << i) && (sys->pic_master.imr & (1 << i)) == 0)
				{
					if (sys->pic_master.isr & (1 << i))
					{
						in_service = 1;
						break;
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