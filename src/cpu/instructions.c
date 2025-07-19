/*This file only include basic instructions
Really only the ones that you can use to do other instructions
mov, push, jmp, sub, add, etc*/

#include "system.h"
#include "memory.h"
#include "cpu/cpu.h"

void mov8(Sys8086* sys, uint8_t* dest, uint8_t* src)
{
	_Bool dest_in_mem = 0;
	_Bool src_in_mem = 0;

	int address_dest = 0;
	int address_src = 0;

	if(dest >= sys->memory && sys->memory + MAX_MEMORY_8086 >= dest)
	{
		address_dest = dest - sys->memory;
		dest_in_mem = 1;
	}

	if(src >= sys->memory && sys->memory + MAX_MEMORY_8086 >= src)
	{
		address_src = src - sys->memory;
		src_in_mem = 1;
	}

	uint8_t final_src = *src;

	if(src_in_mem)
	{
		final_src = read_address8(sys, address_src, 0);
	}

	if(dest_in_mem)
	{
		write_address8(sys, address_dest, final_src, 0);
	}

	else
	{
		*dest = final_src;
	}
}

void mov16(Sys8086* sys, uint16_t* dest, uint16_t* src)
{
	_Bool dest_in_mem = 0;
	_Bool src_in_mem = 0;

	int address_dest = 0;
	int address_src = 0;

	if(dest >= (uint16_t*)sys->memory && (uint16_t*)sys->memory + MAX_MEMORY_8086 >= dest)
	{
		address_dest = dest - (uint16_t*)sys->memory;
		dest_in_mem = 1;
	}

	if(src >= (uint16_t*)sys->memory && (uint16_t*)sys->memory + MAX_MEMORY_8086 >= src)
	{
		address_src = src - (uint16_t*)sys->memory;
		src_in_mem = 1;
	}

	uint16_t final_src = *src;

	if(src_in_mem)
	{
		final_src = read_address16(sys, address_src, 0);
	}

	if(dest_in_mem)
	{
		write_address16(sys, address_dest, final_src, 0);
	}

	else
	{
		*dest = final_src;
	}
}

void push(Sys8086* sys, uint16_t value)
{
	sys->cpu.sp.whole -= 2;
	write_address16(sys, seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole), value, 0);
}

void pop(Sys8086* sys, uint16_t* value)
{
	*value = read_address16(sys, seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole), 0);
	sys->cpu.sp.whole += 2;
}

void jmp(Sys8086* sys, uint16_t segment, uint16_t offset)
{
	sys->cpu.cs.whole = segment;
	sys->cpu.ip.whole = offset;
}

void inc8(Sys8086* sys, uint8_t* value)
{
	uint8_t old_val = *value;

	(*value)++;

	cpu_modify_flag_half_carry(&sys->cpu, old_val, *value);
	cpu_modify_flag_zero(&sys->cpu, *value);
	cpu_modify_flag_parity(&sys->cpu, *value);
	cpu_modify_flag_sign(&sys->cpu, *value, 0);
}

void dec8(Sys8086* sys, uint8_t* value)
{
	uint8_t old_val = *value;

	(*value)--;

	cpu_modify_flag_half_carry(&sys->cpu, old_val, *value);
	cpu_modify_flag_zero(&sys->cpu, *value);
	cpu_modify_flag_parity(&sys->cpu, *value);
	cpu_modify_flag_sign(&sys->cpu, *value, 0);
}

void inc16(Sys8086* sys, uint16_t* value)
{
	uint16_t old_val = *value;

	(*value)++;

	cpu_modify_flag_half_carry(&sys->cpu, old_val, *value);
	cpu_modify_flag_zero(&sys->cpu, *value);
	cpu_modify_flag_parity(&sys->cpu, *value);
	cpu_modify_flag_sign(&sys->cpu, *value, 1);
}

void dec16(Sys8086* sys, uint16_t* value)
{
	uint16_t old_val = *value;

	(*value)--;

	cpu_modify_flag_half_carry(&sys->cpu, old_val, *value);
	cpu_modify_flag_zero(&sys->cpu, *value);
	cpu_modify_flag_parity(&sys->cpu, *value);
	cpu_modify_flag_sign(&sys->cpu, *value, 1);
}

void cmp16(Sys8086* sys, uint16_t val1, uint16_t val2)
{
	uint16_t temp = val1 - val2;

	cpu_modify_flag_carry(&sys->cpu, val1, temp, 1);
	cpu_modify_flag_half_carry(&sys->cpu, val1, temp);
	cpu_modify_flag_zero(&sys->cpu, temp);
	cpu_modify_flag_parity(&sys->cpu, temp);
	cpu_modify_flag_sign(&sys->cpu, temp, 1);
	cpu_modify_flag_overflow(&sys->cpu, val1, val2, temp, 1);
}

void cmp8(Sys8086* sys, uint8_t val1, uint8_t val2)
{
	uint8_t temp = val1 - val2;

	cpu_modify_flag_carry(&sys->cpu, val1, temp, 0);
	cpu_modify_flag_half_carry(&sys->cpu, val1, temp);
	cpu_modify_flag_zero(&sys->cpu, temp);
	cpu_modify_flag_parity(&sys->cpu, temp);
	cpu_modify_flag_sign(&sys->cpu, temp, 0);
	cpu_modify_flag_overflow(&sys->cpu, val1, val2, temp, 0);
}

void mul16(Sys8086* sys, uint16_t value)
{
	uint32_t product = value * sys->cpu.ax.whole;

	sys->cpu.ax.whole = product & 0x0000ffff; // Store the lower bytes in ax
	sys->cpu.dx.whole = (product & 0xffff0000) >> 16; // Store the higher bytes in dx

	// Special flag case, don't use cpu_modify_flag* function
	// If upper bytes are not 0 set the flags
	if (sys->cpu.dx.whole != 0)
	{
		sys->cpu.flag.whole |= FLAG_OVERFLOW;
		sys->cpu.flag.whole |= FLAG_CARRY;
	}

	else
	{
		sys->cpu.flag.whole &= ~FLAG_OVERFLOW;
		sys->cpu.flag.whole &= ~FLAG_CARRY;
	}
}

void mul8(Sys8086* sys, uint8_t value)
{
	uint16_t product = value * sys->cpu.ax.low;

	sys->cpu.ax.whole = product;

	// Special flag case, don't use cpu_modify_flag* function
	// If upper bytes are not 0 set the flags
	if (sys->cpu.ax.whole & 0xff00)
	{
		sys->cpu.flag.whole |= FLAG_OVERFLOW;
		sys->cpu.flag.whole |= FLAG_CARRY;
	}

	else
	{
		sys->cpu.flag.whole &= ~FLAG_OVERFLOW;
		sys->cpu.flag.whole &= ~FLAG_CARRY;
	}
}

void add8(Sys8086* sys, uint8_t* to, uint8_t added)
{
	uint8_t old_val = *to;

	(*to) += added;

	cpu_modify_flag_carry(&sys->cpu, old_val, *to, 0);
	cpu_modify_flag_half_carry(&sys->cpu, old_val, *to);
	cpu_modify_flag_zero(&sys->cpu, *to);
	cpu_modify_flag_parity(&sys->cpu, *to);
	cpu_modify_flag_sign(&sys->cpu, *to, 0);
	cpu_modify_flag_overflow(&sys->cpu, old_val, added, *to, 0);
}

void add16(Sys8086* sys, uint16_t* to, uint16_t added)
{
	uint16_t old_val = *to;

	(*to) += added;

	cpu_modify_flag_carry(&sys->cpu, old_val, *to, 1);
	cpu_modify_flag_half_carry(&sys->cpu, old_val, *to);
	cpu_modify_flag_zero(&sys->cpu, *to);
	cpu_modify_flag_parity(&sys->cpu, *to);
	cpu_modify_flag_sign(&sys->cpu, *to, 1);
	cpu_modify_flag_overflow(&sys->cpu, old_val, added, *to, 1);
}
