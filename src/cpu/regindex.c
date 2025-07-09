#include "system.h"

uint16_t* segment_reg_index(CPU* cpu, int index)
{
	switch (index)
	{
	case 0:
	{
		return &cpu->es.whole;
	}
	case 1:
	{
		return &cpu->cs.whole;
	}
	case 2:
	{
		return &cpu->ss.whole;
	}
	case 3:
	{
		return &cpu->ds.whole;
	}
	}
}

// Takes the index passed and returns the respective register

uint16_t* reg16_index(CPU* cpu, int index)
{
	switch (index)
	{
	case 0:
	{
		return &cpu->ax.whole;
	}
	case 1:
	{
		return &cpu->cx.whole;
	}
	case 2:
	{
		return &cpu->dx.whole;
	}
	case 3:
	{
		return &cpu->bx.whole;
	}
	case 4:
	{
		return &cpu->sp.whole;
	}
	case 5:
	{
		return &cpu->bp.whole;
	}
	case 6:
	{
		return &cpu->si.whole;
	}
	case 7:
	{
		return &cpu->di.whole;
	}
	}
}

uint8_t* reg8_index(CPU* cpu, int index)
{
	switch (index)
	{
	case 0:
	{
		return &cpu->ax.low;
	}
	case 1:
	{
		return &cpu->cx.low;
	}
	case 2:
	{
		return &cpu->dx.low;
	}
	case 3:
	{
		return &cpu->bx.low;
	}
	case 4:
	{
		return &cpu->ax.high;
	}
	case 5:
	{
		return &cpu->cx.high;
	}
	case 6:
	{
		return &cpu->dx.high;
	}
	case 7:
	{
		return &cpu->bx.high;
	}
	}
}
