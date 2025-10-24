#include "system.h"
#include "cpu/instructions.h"
#include "memory.h"

void pic_check_int(Sys8086* sys)
{
	_Bool interrupt_go = 0;

	uint16_t vector_offset = 0;

	if (sys->cpu.flag.whole & FLAG_INTERRUPT)
	{
		if (sys->pic.irr != 0)
		{
			for (int i = 0; i < 8; i++)
			{
				if (sys->pic.isr & (1 << i))
				{
					break;
				}

				if (sys->pic.irr & (1 << i) && (sys->pic.imr & (1 << i)) == 0)
				{
					sys->pic.isr |= (1 << i);
					sys->pic.irr &= ~(1 << i);

					vector_offset = sys->pic.vector_offset * 4 + i * 4;
					interrupt_go = 1;
					break;
				}
			}

			if (interrupt_go)
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