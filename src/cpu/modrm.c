#include "system.h"

// Returns instruction pointer advancement
// Takes the sys, data segment that mem will offset to, the instruction address, the reg, the rm (regmem), an immediate (NULL if none), word is if regmem/reg is a word, imm_word if the imm is a word or not, sreg if reg is a segment register

// mod rm byte

// 00 000 000
// mod reg rm

uint8_t calc_modrm_byte(Sys8086* sys, Register* data_seg, int instruction_address, void** reg, void** regmem, void* imm, _Bool word, _Bool imm_word, _Bool sreg)
{
	uint8_t modrm = sys->memory[instruction_address + 1];

	// modrm == 0b10'000'000

	uint8_t mod_val = (modrm & 0b11'000'000) >> 6;
	uint8_t reg_val = (modrm & 0b00'111'000) >> 3;
	uint8_t rm_val = modrm & 0b00'000'111;

	int imm_position = 0;
	int displacement_position = 0;

	// No displacement
	if ((mod_val == 0b00 && rm_val != 6) || mod_val == 0b11)
	{
		displacement_position = -1; // not used
		imm_position = 2;
	}

	// 8 bit displacement
	else if (mod_val == 0b01)
	{
		displacement_position = 2;
		imm_position = 3;
	}

	// 16 bit displacement

	else if (mod_val == 0b10 || (mod_val == 0b00 && rm_val == 6))
	{
		displacement_position = 2;
		imm_position = 4;
	}

	// Using mem, not reg in regmem
	if (mod_val == 0b01 || mod_val == 0b10 || mod_val == 0b00) // No displacement for mod 0 (except rm_val == 6)
	{
		*regmem = 0;

		// Do we also have a immediate byte/word? then we can find that out to and give it to the caller

		if (imm != NULL)
		{
			if (imm_word)
			{
				uint16_t* imm_ptr = &sys->memory[instruction_address + imm_position];

				*(uint16_t*)imm = *imm_ptr;
			}

			else
			{
				*(uint8_t*)imm = sys->memory[instruction_address + imm_position];
			}
		}

		// Add displacement to regmem first before adding (if displacement provided)

		if (mod_val == 0b01)
		{
			*(uint8_t*)regmem += sys->memory[instruction_address + displacement_position];
		}

		else if (mod_val == 0b10)
		{
			uint16_t* displacement_ptr = &sys->memory[instruction_address + displacement_position];

			*(uint16_t*)regmem += *displacement_ptr;
		}

		// Add registers to rmmem

		switch (rm_val)
		{
		case 0:
		{
			*(uint16_t*)regmem += sys->cpu.bx.whole + sys->cpu.si.whole;
			break;
		}
		case 1:
		{
			*(uint16_t*)regmem += sys->cpu.bx.whole + sys->cpu.di.whole;
			break;
		}
		case 2:
		{
			*(uint16_t*)regmem += sys->cpu.bp.whole + sys->cpu.si.whole;
			break;
		}
		case 3:
		{
			*(uint16_t*)regmem += sys->cpu.bp.whole + sys->cpu.di.whole;
			break;
		}
		case 4:
		{
			*(uint16_t*)regmem += sys->cpu.si.whole;
			break;
		}
		case 5:
		{
			*(uint16_t*)regmem += sys->cpu.di.whole;
			break;
		}
		case 6:
		{
			if (mod_val == 0)
			{
				uint16_t* displacement_ptr = &sys->memory[instruction_address + displacement_position];

				*(uint16_t*)regmem += *displacement_ptr;
			}

			else
			{
				*(uint16_t*)regmem += sys->cpu.bp.whole;
			}

			break;
		}
		case 7:
		{
			*(uint16_t*)regmem += sys->cpu.bx.whole;
			break;
		}
		}

		void* real_regmem_memory_address = &(sys->memory[seg_mem(data_seg->whole, *regmem)]);
		*regmem = real_regmem_memory_address;
	}

	// rm is same as reg, using rmreg

	if (mod_val == 0b11)
	{
		if (imm != NULL)
		{
			if (imm_word)
			{
				uint16_t* imm_ptr = &sys->memory[instruction_address + imm_position];

				*(uint16_t*)imm = *imm_ptr;
			}

			else
			{
				*(uint8_t*)imm = sys->memory[instruction_address + imm_position];
			}
		}

		if (word)
		{
			*regmem = reg16_index(&sys->cpu, rm_val);
		}

		else
		{
			*regmem = reg8_index(&sys->cpu, rm_val);
		}
	}

	// Determine reg 

	if (sreg)
	{
		*reg = segment_reg_index(&sys->cpu, reg_val);
	}

	else if (word)
	{
		*reg = reg16_index(&sys->cpu, reg_val);
	}

	else
	{
		*reg = reg8_index(&sys->cpu, reg_val);
	}

	int ip_increase = 0;

	// Immediate is the last byte/word

	ip_increase += imm_position;

	if (imm_word && imm != NULL)
	{
		ip_increase += 2;
	}

	else if (imm != NULL)
	{
		ip_increase++;
	}

	return ip_increase; // Amount of how instruction pointer should go upasd
}