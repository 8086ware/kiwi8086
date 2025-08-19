#include "cpu/cpu.h"

void cpu_reset(CPU* cpu)
{
	cpu->cs.whole = 0xFFFF;
	cpu->ip.whole = 0x0;
}
