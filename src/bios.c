#include "bios.h"
#include "system.h"

void bios_int_0x10(Sys8086* sys)
{
	switch (sys->cpu.ax.high)
	{
	case 0x02:
	{
		sys->cpu.ax.whole = 0;
		sys->display.cursor_x[sys->cpu.bx.high] = sys->cpu.dx.low;
		sys->display.cursor_y[sys->cpu.bx.high] = sys->cpu.dx.high;
		break;
	}
	case 0x09:
	{
		for (int i = 0; i < sys->cpu.cx.whole; i++)
		{
			sys->display.mem[(sys->display.cursor_y[sys->cpu.bx.high] * sys->display.columns + sys->display.cursor_x[sys->cpu.bx.high]) * (sys->cpu.bx.high + 1)].disp = sys->cpu.ax.low;
			sys->display.mem[(sys->display.cursor_y[sys->cpu.bx.high] * sys->display.columns + sys->display.cursor_x[sys->cpu.bx.high]) * (sys->cpu.bx.high + 1)].attrib = sys->cpu.bx.low;
		}
		break;
	}
	}
}	