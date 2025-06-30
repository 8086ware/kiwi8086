#include "system.h"

// A 20 bit value but only can be stored in 32 bit 

/* A mod r / m byte is encoded like this:
/ 00   000   000 (binary)
/ mod  reg   r/m
*/

// mod r/m byte, reg, r/m (reg2 and mem)

uint16_t* segment_reg_index(Sys8086* sys, int index)
{
	switch (index)
	{
	case 0:
	{
		return &sys->cpu.es.whole;
	}
	case 1:
	{
		return &sys->cpu.cs.whole;
	}
	case 2:
	{
		return &sys->cpu.ss.whole;
	}
	case 3:
	{
		return &sys->cpu.ds.whole;
	}
	}
}

// Takes the index passed and returns the respective register

uint16_t* reg16_index(Sys8086* sys, int index)
{
	switch (index)
	{
	case 0:
	{
		return &sys->cpu.ax.whole;
	}
	case 1:
	{
		return &sys->cpu.cx.whole;
	}
	case 2:
	{
		return &sys->cpu.dx.whole;
	}
	case 3:
	{
		return &sys->cpu.bx.whole;
	}
	case 4:
	{
		return &sys->cpu.sp.whole;
	}
	case 5:
	{
		return &sys->cpu.bp.whole;
	}
	case 6:
	{
		return &sys->cpu.si.whole;
	}
	case 7:
	{
		return &sys->cpu.di.whole;
	}
	}
}

uint8_t* reg8_index(Sys8086* sys, int index)
{
	switch (index)
	{
	case 0:
	{
		return &sys->cpu.ax.low;
	}
	case 1:
	{
		return &sys->cpu.cx.low;
	}
	case 2:
	{
		return &sys->cpu.dx.low;
	}
	case 3:
	{
		return &sys->cpu.bx.low;
	}
	case 4:
	{
		return &sys->cpu.ax.high;
	}
	case 5:
	{
		return &sys->cpu.cx.high;
	}
	case 6:
	{
		return &sys->cpu.dx.high;
	}
	case 7:
	{
		return &sys->cpu.bx.high;
	}
	}
}

// Returns reg_val because opcode groups depend on it
// System, the mod r/m byte, the receiving register, the receiving rm register, and the recieving rmmem address. Depends on the mod r/m byte
// if you are gonna use rmreg or rmmem. As well as reg could equal an instruction identifier when dealing with an opcode group.
// Rmreg is null, or rmmem is -1 depending on whats used
// Displacement is for the displacement memory if rmmem is used. 
// Word is 1 for 16bit operations and 0 for 8bit.
// Sreg is for dealing with segment registers

uint8_t calc_modrm_byte(Sys8086* sys, Register* data_seg, int instruction_address, void** reg, void** regmem, void* imm, _Bool word, _Bool imm_byte, _Bool sreg)
{
	uint8_t modrm = sys->memory[instruction_address + 1];

	uint8_t mod_val = (modrm & 0b11'000'000) >> 6;
	uint8_t reg_val = (modrm & 0b00'111'000) >> 3;
	uint8_t rm_val = modrm & 0b00'000'111;

	int imm_position = 0;
	int displacement_position = 0;

	if ((mod_val == 0b00 && rm_val != 6) || mod_val == 0b11)
	{
		displacement_position = -1; // not used
		imm_position = 2;
	}

	else if (mod_val == 0b01)
	{
		displacement_position = 2;
		imm_position = 3;
	}

	else if(mod_val == 0b10 || (mod_val == 0b00 && rm_val == 6))
	{
		displacement_position = 2;
		imm_position = 4;
	}

	// Using mem, not reg in regmem
	if (mod_val == 0b01 || mod_val == 0b10 || mod_val == 0b00) // No displacement for mod 0 (except rm_val == 6)
	{
		*regmem = 0;

		// Do we also have a immediate byte/word? then we can find that out to and give it to the caller

		if (imm_byte)
		{
			if (word)
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

		else if(mod_val == 0b10)
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

				*(uint8_t*)regmem += *displacement_ptr;
			}

			else
			{
				*(uint8_t*)regmem += sys->cpu.bp.whole;
			}

			break;
		}
		case 7:
		{
			*(uint8_t*)regmem += sys->cpu.bx.whole;
			break;
		}
		}

		void* real_regmem_memory_address = &(sys->memory[seg_mem(data_seg->whole, *regmem)]);
		*regmem = real_regmem_memory_address;
	}

	// rm is same as reg, using rmreg

	if (mod_val == 0b11)
	{
		if (imm_byte)
		{
			if (word)
			{
				uint16_t* imm_ptr = &sys->memory[instruction_address + imm_position];

				*(uint16_t*)imm = *imm_ptr;
			}

			else
			{
				*(uint8_t*)imm = sys->memory[instruction_address + imm_position];
			}
		}

		*rmmem = -1; // Memory address is -1 (invalid) if we are using rmreg

		if (word)
		{
			*rmreg = reg16_index(sys, rm_val);
		}

		else
		{
			*regmem = reg8_index(sys, rm_val);
		}
	}

	// Determine reg 

	if (sreg)
	{
		*reg = segment_reg_index(sys, reg_val);
	}

	else if (word)
	{
		*reg = reg16_index(sys, reg_val);
	}

	else
	{
		*reg = reg8_index(sys, reg_val);
	}

	int ip_increase = 0; 

	if (mod_val == 0b01 || mod_val == 0b10 || mod_val == 0b00)
	{
		// Immediate is the last byte/word
		if (imm_byte)
		{
			ip_increase += imm_position;
}

		if (word)
		{
			ip_increase += 2;
		}

		else
		{
			ip_increase++;
		}
	}

	else
	{
		ip_increase += 2;
	}

	return ip_increase; // Amount of how instruction pointer should go upasd
}

uint32_t seg_mem(uint16_t seg, uint16_t offset)
{
	return seg * 0x10 + offset;
}

void cpu_exec(Sys8086* sys)
{
	Register* data_seg = &sys->cpu.ds;
	Register* dest_seg = &sys->cpu.es;

	int cur_inst = seg_mem(sys->cpu.cs.whole, sys->cpu.ip.whole);

	// Only used if instruction has mod r/m byte
	// We don't know if the registers we are accesing are 8 bit or 16 bit so cast them when dealing with them
	void* reg = NULL; 
	void* rmreg = NULL;
	int rm_mem_addr = 0;

	uint8_t opcode = sys->memory[cur_inst];

	switch (sys->memory[cur_inst - 1]) // opcode prefix
	{
	case PREFIX_ES:
	{
		data_seg = &sys->cpu.es;
		break;
	}
	case PREFIX_CS:
	{
		data_seg = &sys->cpu.cs;
		break;
	}
	case PREFIX_SS:
	{
		data_seg = &sys->cpu.ss;
		break;
	}
	case PREFIX_DS:
	{
		data_seg = &sys->cpu.ds;
		break;
	}
	}

	// In the opcodes, dd is displacement and ii is immediate and mm is mod. All of them are optional
	
	switch (opcode) // actual opcode
	{
	case GROUP_OPCODE_FF:
	{
		uint8_t group_opcode_instruction = (sys->memory[cur_inst + 1] & 0b00111000) >> 3; // reg part identifies the opcode in mod rm byte

		switch (group_opcode_instruction)
		{
		case JMP_RM16: // FF mm ii ii q
		{
			calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 0);
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];

			if (rm_mem_addr != -1)
			{
				sys->cpu.ip.whole = *(uint16_t*)rmmem;
			}

			else
			{
				sys->cpu.ip.whole = *(uint16_t*)rmreg;
			}

			break;
		}

		case PUSH_RM16: // FF mm dd dd
		{
			calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 0);
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];

			sys->cpu.sp.whole -= 2;

			uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

			if (rm_mem_addr != -1)
			{
				*stack = *(uint16_t*)rmmem;
				sys->cpu.ip.whole += 4;
			}

			else
			{
				*stack = *(uint16_t*)rmreg;
				sys->cpu.ip.whole += 2;
			}

			break;
		}
		}

		break;
	}
	case ADD_AL_IMM8:
	{
		sys->cpu.ax.low += sys->memory[cur_inst + 1];
		sys->cpu.ip.whole += 2;
		break;
	}
	case ADD_AX_IMM16:
	{
		uint16_t* imm16 = &sys->memory[cur_inst + 1];
		sys->cpu.ax.whole += *imm16;
		sys->cpu.ip.whole += 3;
		break;
	}
	case MOV_RM8_R8: // 88 mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 0, 0, 0);

		if (rm_mem_addr != -1)
		{
			sys->memory[seg_mem(data_seg->whole, rm_mem_addr)] = *(uint8_t*)reg;
			sys->cpu.ip.whole += 4;
		}

		else
		{
			*(uint8_t*)rmreg = *(uint8_t*)reg;
			sys->cpu.ip.whole += 2;
		}

		break;
	}
	case MOV_RM16_R16: // 89 mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 0);

		uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];

		if (rm_mem_addr != -1)
		{
			*rmmem = *(uint16_t*)reg;
			sys->cpu.ip.whole += 4;
		}

		else
		{
			*(uint16_t*)rmreg = *(uint16_t*)reg;
			sys->cpu.ip.whole += 2;
		}

		break;
	}
	case MOV_R8_RM8: // 8A mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 0, 0, 0);

		if (rm_mem_addr != -1)
		{
			*(uint8_t*)reg = sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];
			sys->cpu.ip.whole += 4;
		}

		else
		{
			*(uint8_t*)reg = *(uint8_t*)rmreg;
			sys->cpu.ip.whole += 2;
		}

		break;
	}
	case MOV_R16_RM16: // 8B mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 0);

		if (rm_mem_addr != -1)
		{
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];
			*(uint16_t*)reg = *(uint16_t*)rmmem;
			sys->cpu.ip.whole += 4;
		}

		else
		{
			*(uint16_t*)reg = *(uint16_t*)rmreg;
			sys->cpu.ip.whole += 2;
		}

		break;
	}
	case MOV_RM16_SREG: // 8C mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 1);

		if (rm_mem_addr != -1)
		{
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];
			*(uint16_t*)rmmem = *(uint16_t*)reg;
			sys->cpu.ip.whole += 4;
		}

		else
		{
			*(uint16_t*)rmreg = *(uint16_t*)reg;
			sys->cpu.ip.whole += 2;
		}

		break;
	}
	case MOV_REG16_IMM16: // B8+x ii ii
	case MOV_REG16_IMM16 + 1:
	case MOV_REG16_IMM16 + 2:
	case MOV_REG16_IMM16 + 3:
	case MOV_REG16_IMM16 + 4:
	case MOV_REG16_IMM16 + 5:
	case MOV_REG16_IMM16 + 6:
	case MOV_REG16_IMM16 + 7:
	{
		int reg_code = opcode - MOV_REG16_IMM16;

		reg = reg16_index(sys, reg_code);

		uint16_t* imm16 = &sys->memory[cur_inst + 1];
		*(uint16_t*)reg = *imm16;

		sys->cpu.ip.whole += 3;

		break;
	}
	case MOV_SREG_RM16: // 8E mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 1);

		if (rm_mem_addr != -1)
		{
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];
			*(uint16_t*)reg = *rmmem;
			sys->cpu.ip.whole += 4;
		}

		else
		{
			*(uint16_t*)reg = *(uint16_t*)rmreg;
			sys->cpu.ip.whole += 2;
		}

		break;
	}
	case MOV_AL_MOFFS8: // A0 dd dd
	{
		uint16_t* moffs8 = &sys->memory[cur_inst + 1];

		sys->cpu.ax.low = sys->memory[seg_mem(data_seg->whole, *moffs8)];

		sys->cpu.ip.whole += 3;
		break;
	}
	case MOV_AX_MOFFS16: // A1 dd dd
	{
		uint16_t* moffs16 = &sys->memory[cur_inst + 1];
		uint16_t* moffs16_real = &sys->memory[seg_mem(data_seg->whole, *moffs16)];

		sys->cpu.ax.whole = *moffs16_real;

		sys->cpu.ip.whole += 3;
		break;
	}
	case MOV_RM8_IMM8: // C6 mm dd dd ii
	{
		int imm8 = 0;

		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, &imm8, 0, 1, 0);

		if (rm_mem_addr != -1)
		{
			sys->memory[seg_mem(data_seg->whole, rm_mem_addr)] = (uint8_t)imm8;
			sys->cpu.ip.whole += 5;
		}

		else
		{
			*(uint8_t*)rmreg = imm8;
			sys->cpu.ip.whole += 3;
		}

		break;
	}
	case MOV_RM16_IMM16: // C7 mm dd dd ii ii
	{
		int imm16 = 0;

		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, &imm16, 1, 1, 0);

		if (rm_mem_addr != -1)
		{
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];
			*rmmem = (uint16_t)imm16;
			sys->cpu.ip.whole += 6;
		}

		else
		{
			*(uint16_t*)rmreg = imm16;
			sys->cpu.ip.whole += 4;
		}

		break;
	}
	case JMP_REL8: // EB ii
	{
		sys->cpu.ip.whole += (int8_t)sys->memory[cur_inst + 1] + 2;
		break;
	}
	case JMP_REL16: // E9 ii ii
	{
		int16_t* jmp_value = &sys->memory[cur_inst + 1];

		sys->cpu.ip.whole += *jmp_value + 3;
		break;
	}
	case JMP_PTR16_16: // EA ii ii ii ii
	{
		uint16_t* new_ip = &sys->memory[cur_inst + 1];
		uint16_t* new_cs = &sys->memory[cur_inst + 3];

		sys->cpu.ip.whole = *new_ip;
		sys->cpu.cs.whole = *new_cs;

		break;
	}
	case PUSH_R16: // 50 + i
	case PUSH_R16 + 1:
	case PUSH_R16 + 2:
	case PUSH_R16 + 3:
	case PUSH_R16 + 4:
	case PUSH_R16 + 5:
	case PUSH_R16 + 6:
	case PUSH_R16 + 7:
	{
		sys->cpu.sp.whole -= 2;

		int reg_code = opcode - PUSH_R16;

		reg = reg16_index(sys, reg_code);

		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*stack = *(uint16_t*)reg;

		sys->cpu.ip.whole++;
		break;
	}
	// 0x06
	case PUSH_SREG: // es
	case PUSH_SREG + 0x8: // cs
	case PUSH_SREG + 0x10: // ss
	case PUSH_SREG + 0x18: // ds
	{
		sys->cpu.sp.whole -= 2;

		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		Register* sreg = NULL;

		if ((opcode - 0x8) == POP_SREG)
		{
			sreg = &sys->cpu.cs.whole;
		}

		else if ((opcode - 0x10) == POP_SREG)
		{
			sreg = &sys->cpu.ss.whole;
		}

		else if ((opcode - 0x18) == POP_SREG)
		{
			sreg = &sys->cpu.ds.whole;
		}

		else
		{
			sreg = &sys->cpu.es.whole;
		}

		*stack = sreg->whole;

		sys->cpu.ip.whole++;
		break;
	}
	case POP_RM16: // 8F mm dd dd
	{
		calc_modrm_byte(sys, cur_inst, &reg, &rmreg, &rm_mem_addr, NULL, 1, 0, 0);
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		if (rm_mem_addr != -1)
		{
			uint16_t* rmmem = &sys->memory[seg_mem(data_seg->whole, rm_mem_addr)];
			*rmmem = *stack;
			sys->cpu.ip.whole += 4;
		}
		
		else
		{
			*(uint16_t*)rmreg = *stack;
			sys->cpu.ip.whole += 2;
		}

		sys->cpu.sp.whole += 2;

		break;
	}
	case POP_R16: // 58 + i
	case POP_R16 + 1:
	case POP_R16 + 2:
	case POP_R16 + 3:
	case POP_R16 + 4:
	case POP_R16 + 5:
	case POP_R16 + 6:
	case POP_R16 + 7:
	{
		int reg_code = opcode - POP_R16;

		reg = reg16_index(sys, reg_code);

		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*(uint16_t*)reg = *stack;

		sys->cpu.sp.whole += 2;
		sys->cpu.ip.whole += 2;
		break;
	}
	// 0x07
	case POP_SREG: // es
	case POP_SREG + 0x8: // cs
	case POP_SREG + 0x10: // ss
	case POP_SREG + 0x18: // ds
	{
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		Register* sreg = NULL;

		if ((opcode - 0x8) == POP_SREG)
		{
			sreg = &sys->cpu.cs.whole;
		}

		else if ((opcode - 0x10) == POP_SREG)
		{
			sreg = &sys->cpu.ss.whole;
		}

		else if ((opcode - 0x18) == POP_SREG)
		{
			sreg = &sys->cpu.ds.whole;
		}

		else
		{
			sreg = &sys->cpu.es.whole;
		}

		sreg->whole = *stack;

		sys->cpu.sp.whole += 2;
		sys->cpu.ip.whole++;
		break;
	}
	default:
		sys->cpu.ip.whole++;
		break;
	}
}