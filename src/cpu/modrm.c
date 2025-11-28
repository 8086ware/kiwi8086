#include "system.h"
#include "memory.h"

// Returns instruction pointer advancement
// Takes the sys, data segment that mem will offset to, the instruction address, the reg, the rm (regmem), an immediate (NULL if none), word is if regmem/reg is a word, imm_word if the imm is a word or not, sreg if reg is a segment register

// mod rm byte

// 00 000 000
// mod reg rm
// fill in instruction before calcing
void calc_modrm_byte(Sys8086* sys, Instruction* instruction, int modrm_address, _Bool sreg)
{
	instruction->modrm = 1;

	_Bool default_seg = 1;

	// Im not rewriting on calc_modrm_byte instances, so in cpu_exec we create a temporary variable with the value of DS
	// (default segment) and if it actually POINTS to DS or any other segment, then use that

	if (instruction->data_seg == &sys->cpu.es || instruction->data_seg == &sys->cpu.cs || instruction->data_seg == &sys->cpu.ss || instruction->data_seg == &sys->cpu.ds)
	{
		default_seg = 0;
	}

	else
	{
		default_seg = 1;
		instruction->data_seg = &sys->cpu.ds;
	}

	uint8_t modrm = read_address8(sys, modrm_address, 0);

	// modrm == 0b10'000'000

	uint8_t mod_val = (modrm & 0b11000000) >> 6;
	uint8_t reg_val = (modrm & 0b00111000) >> 3;
	uint8_t rm_val = modrm & 0b00000111;

	int16_t displacement16 = 0;
	int8_t displacement8 = 0;

	if (mod_val == 0b01)
	{
		displacement8 = (int8_t)read_address8(sys, modrm_address + 1, 0);
		instruction->length += 2;
	}

	else if(mod_val == 0b10 || (mod_val == 0b00 && rm_val == 6))
	{
		displacement16 = (int16_t)read_address16(sys, modrm_address + 1, 0);
		instruction->length += 3;
	}

	else
	{
		instruction->length++;
	}

	uint16_t temp_regmem = 0;

	// Using mem, not reg in regmem
	if (mod_val == 0b01 || mod_val == 0b10 || mod_val == 0b00) // No displacement for mod 0 (except rm_val == 6)
	{
		instruction->regmem = 0;

		switch (rm_val)
		{
		case 0:
		{
			temp_regmem += sys->cpu.bx.whole + sys->cpu.si.whole;
			break;
		}
		case 1:
		{
			temp_regmem += sys->cpu.bx.whole + sys->cpu.di.whole;
			break;
		}
		case 2:
		{
			if (default_seg)
			{
				instruction->data_seg = &sys->cpu.ss;
			}

			temp_regmem += sys->cpu.bp.whole + sys->cpu.si.whole;
			break;
		}
		case 3:
		{
			if (default_seg)
			{
				instruction->data_seg = &sys->cpu.ss;
			}

			temp_regmem += sys->cpu.bp.whole + sys->cpu.di.whole;
			break;
		}
		case 4:
		{
			temp_regmem += sys->cpu.si.whole;
			break;
		}
		case 5:
		{
			temp_regmem += sys->cpu.di.whole;
			break;
		}
		case 6:
		{
			if (mod_val == 0)
			{
				temp_regmem += displacement16;
			}

			else
			{
				if (default_seg)
				{
					instruction->data_seg = &sys->cpu.ss;
				}

				temp_regmem += sys->cpu.bp.whole;
			}

			break;
		}
		case 7:
		{
			temp_regmem += sys->cpu.bx.whole;
			break;
		}
		}

		// Add displacement to regmem first before adding (if displacement provided)

		if (mod_val == 0b01)
		{
			temp_regmem += displacement8;
		}

		else if (mod_val == 0b10)
		{
			temp_regmem += displacement16;
		}

		instruction->regmem = &sys->memory[seg_mem(instruction->data_seg->whole, temp_regmem)];
	}

	// rm is same as reg, using rmreg

	else if (mod_val == 0b11)
	{
		if (instruction->width)
		{
			instruction->regmem = reg16_index(&sys->cpu, rm_val);
		}

		else
		{
			instruction->regmem = reg8_index(&sys->cpu, rm_val);
		}
	}

	// Determine reg 

	if (sreg)
	{
		instruction->reg = segment_reg_index(&sys->cpu, reg_val);
	}

	else if (instruction->width)
	{
		instruction->reg = reg16_index(&sys->cpu, reg_val);
	}

	else
	{
		instruction->reg = reg8_index(&sys->cpu, reg_val);
	}
}
