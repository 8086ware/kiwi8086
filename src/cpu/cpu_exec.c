#include "system.h"
#include "bios.h"

void cpu_exec(Sys8086* sys)
{
	if (sys->pic_slave.irq != -1) // Give the request to the master pic
	{
		sys->pic_master.irq = sys->pic_slave.irq;
	}

	int irq_vector_offset = 0;

	if (sys->pic_master.irq != -1)
	{
		if (sys->pic_master.irq <= 7)
		{
			irq_vector_offset = sys->pic_master.irq + sys->pic_master.vector_offset;
		}

		else
		{
			irq_vector_offset = sys->pic_master.irq + sys->pic_slave.vector_offset;
		}
	}

	int ip_increase = 0;

	Register* data_seg = &sys->cpu.ds;
	Register* dest_seg = &sys->cpu.es;

	int cur_inst = seg_mem(sys->cpu.cs.whole, sys->cpu.ip.whole);

	// Only used if instruction has mod r/m byte
	// We don't know if the registers we are accesing are 8 bit or 16 bit so cast them when dealing with them
	void* reg = NULL; 
	void* regmem = NULL;

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
	uint8_t group_opcode_instruction = (sys->memory[cur_inst + 1] & 0b00111000) >> 3; // reg part identifies the opcode in mod rm byte

	switch (opcode) // actual opcode
	{
	case GROUP_OPCODE_80:
	{
		switch (group_opcode_instruction)
		{
		case CMP_RM8_IMM8: // 80 mm ii
		{
			uint8_t imm = 0;
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

			uint8_t temp = *(uint8_t*)regmem - imm;

			cpu_modify_flag_carry(sys, *(uint8_t*)regmem, temp, 0);
			cpu_modify_flag_half_carry(sys, *(uint8_t*)regmem, temp);
			cpu_modify_flag_zero(sys, temp);
			cpu_modify_flag_parity(sys, temp);
			cpu_modify_flag_sign(sys, temp, 0);
			cpu_modify_flag_overflow(sys, *(uint8_t*)regmem, imm, temp, 0);

			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_81:
	{
		switch (group_opcode_instruction)
		{
		case CMP_RM16_IMM16: // 81 mm ii ii
		{
			uint16_t imm = 0;
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

			uint16_t temp = *(uint16_t*)regmem - imm;

			cpu_modify_flag_carry(sys, *(uint16_t*)regmem, temp, 1);
			cpu_modify_flag_half_carry(sys, *(uint16_t*)regmem, temp);
			cpu_modify_flag_zero(sys, temp);
			cpu_modify_flag_parity(sys, temp);
			cpu_modify_flag_sign(sys, temp, 1);
			cpu_modify_flag_overflow(sys, *(uint16_t*)regmem, imm, temp, 1);

			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_83:
	{
		switch (group_opcode_instruction)
		{
		case CMP_RM16_IMM8: // 81 mm ii
		{
			uint16_t imm = 0;
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

			uint16_t temp = *(uint16_t*)regmem - imm;

			cpu_modify_flag_carry(sys, *(uint16_t*)regmem, temp, 1);
			cpu_modify_flag_half_carry(sys, *(uint16_t*)regmem, temp);
			cpu_modify_flag_zero(sys, temp);
			cpu_modify_flag_parity(sys, temp);
			cpu_modify_flag_sign(sys, temp, 1);
			cpu_modify_flag_overflow(sys, *(uint16_t*)regmem, imm, temp, 1);

			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_F6:
	{
		switch (group_opcode_instruction)
		{
		case MUL_RM8: // F6 mm
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

			uint16_t value = *(uint8_t*)regmem * sys->cpu.ax.low;

			sys->cpu.ax.whole = value;

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

			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_F7:
	{
		switch (group_opcode_instruction)
		{
		case MUL_RM16: // F7 mm
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

			uint32_t value = *(uint16_t*)regmem * sys->cpu.ax.whole;

			sys->cpu.ax.whole = value & 0x0000ffff; // Store the lower bytes in ax
			sys->cpu.dx.whole = (value & 0xffff0000) >> 16; // Store the higher bytes in dx

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

			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_FE:
	{
		switch (group_opcode_instruction)
		{
		case DEC_RM8: // FE mm dd dd
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

			uint8_t old_val = (*(uint8_t*)regmem);

			(*(uint8_t*)regmem)--;

			cpu_modify_flag_half_carry(sys, old_val, (*(uint8_t*)regmem));
			cpu_modify_flag_zero(sys, (*(uint8_t*)regmem));
			cpu_modify_flag_parity(sys, (*(uint8_t*)regmem));
			cpu_modify_flag_sign(sys, (*(uint8_t*)regmem), 1);

			break;
		}
		case INC_RM8: // FE mm dd dd
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

			uint8_t old_val = (*(uint8_t*)regmem);

			(*(uint8_t*)regmem)++;

			cpu_modify_flag_half_carry(sys, old_val, (*(uint8_t*)regmem));
			cpu_modify_flag_zero(sys, (*(uint8_t*)regmem));
			cpu_modify_flag_parity(sys, (*(uint8_t*)regmem));
			cpu_modify_flag_sign(sys, (*(uint8_t*)regmem), 0);

			break;
		}
		}

		break;
	}
	case GROUP_OPCODE_FF:
	{
		switch (group_opcode_instruction)
		{
		case DEC_RM16: // FF mm dd dd
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

			uint16_t old_val = (*(uint16_t*)regmem);

			(*(uint16_t*)regmem)--;

			break;
		}
		// 0x2
		case CALL_RM16: // FF mm dd dd
		{
			sys->cpu.sp.whole -= 2;
			uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

			*stack = sys->cpu.ip.whole;

			calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

			sys->cpu.ip.whole = *(uint16_t*)regmem;

			break;
		}
		// 0x3
		case CALL_M16_16: // FF mm
		{
			sys->cpu.sp.whole -= 2;
			uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];
			*stack = sys->cpu.cs.whole;
			sys->cpu.sp.whole -= 2;
			stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];
			*stack = sys->cpu.ip.whole;

			calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

			sys->cpu.cs.whole = *(uint16_t*)regmem;
			((uint16_t*)regmem)++;
			sys->cpu.ip.whole = *(uint16_t*)regmem;

			break;
		}
		// 0x4
		case JMP_RM16: // FF mm dd dd
		{
			calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

			sys->cpu.ip.whole = *(uint16_t*)regmem;

			break;
		}
		// 0x6
		case PUSH_RM16: // FF mm dd dd
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
			sys->cpu.sp.whole -= 2;

			uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

			*stack = *(uint16_t*)regmem;

			break;
		}
		// 0x0
		case INC_RM16: // FF mm dd dd
		{
			ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

			uint16_t old_val = (*(uint16_t*)regmem);

			(*(uint16_t*)regmem)++;

			cpu_modify_flag_half_carry(sys, old_val, (*(uint16_t*)regmem));
			cpu_modify_flag_zero(sys, (*(uint16_t*)regmem));
			cpu_modify_flag_parity(sys, (*(uint16_t*)regmem));
			cpu_modify_flag_sign(sys, (*(uint16_t*)regmem), 1);
	
			break;
		}
		}

		break;
	}
	case ADD_AL_IMM8:
	{
		uint8_t old_val = sys->cpu.ax.low;
		sys->cpu.ax.low += sys->memory[cur_inst + 1];
		ip_increase = 2;
 
		cpu_modify_flag_carry(sys, old_val, sys->cpu.ax.low, 0);
		cpu_modify_flag_half_carry(sys, old_val, sys->cpu.ax.low);
		cpu_modify_flag_zero(sys, sys->cpu.ax.low);
		cpu_modify_flag_parity(sys, sys->cpu.ax.low);
		cpu_modify_flag_sign(sys, sys->cpu.ax.low, 0);

		break;
	}
	case ADD_AX_IMM16:
	{
		uint16_t* imm16 = &sys->memory[cur_inst + 1];
		sys->cpu.ax.whole += *imm16;
		ip_increase = 3;
		break;
	}
	case CALL_PTR16_16: // 9A ii ii ii ii
	{
		sys->cpu.sp.whole -= 2;
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];
		*stack = sys->cpu.cs.whole;
		sys->cpu.sp.whole -= 2;
		stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];
		*stack = sys->cpu.ip.whole + 6;

		uint16_t* ptr16 = &sys->memory[cur_inst + 1];
		uint16_t* offset16 = &sys->memory[cur_inst + 3];

		sys->cpu.cs.whole = *ptr16;
		sys->cpu.ip.whole = *offset16;

		break;
	}
	case CALL_REL16: // E8 ii ii
	{
		sys->cpu.sp.whole -= 2;
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*stack = sys->cpu.ip.whole + 3;

		int16_t* call_value = &sys->memory[cur_inst + 1];

		ip_increase = *call_value + 3;
		break;
	}
	case CMP_AL_IMM8: // 3C ii
	{
		uint8_t temp = sys->cpu.ax.low - sys->memory[cur_inst + 1];

		cpu_modify_flag_carry(sys, sys->cpu.ax.low, temp, 0);
		cpu_modify_flag_half_carry(sys, sys->cpu.ax.low, temp);
		cpu_modify_flag_zero(sys, temp);
		cpu_modify_flag_parity(sys, temp);
		cpu_modify_flag_sign(sys, temp, 0);
		cpu_modify_flag_overflow(sys, sys->cpu.ax.low, sys->memory[cur_inst + 1], temp, 0);

		ip_increase = 2;
		break;
	}
	case CMP_AX_IMM16: // 3D ii ii
	{
		uint16_t* imm16 = &sys->memory[cur_inst + 1];
		uint16_t temp = sys->cpu.ax.whole - *imm16;

		cpu_modify_flag_carry(sys, sys->cpu.ax.whole, temp, 1);
		cpu_modify_flag_half_carry(sys, sys->cpu.ax.whole, temp);
		cpu_modify_flag_zero(sys, temp);
		cpu_modify_flag_parity(sys, temp);
		cpu_modify_flag_sign(sys, temp, 1);
		cpu_modify_flag_overflow(sys, sys->cpu.ax.whole, *imm16, temp, 1);

		ip_increase = 3;
		break;
	}
	case CMP_RM8_R8: // 38 mm
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

		uint8_t temp = *(uint8_t*)regmem - *(uint8_t*)reg;

		cpu_modify_flag_carry(sys, *(uint8_t*)regmem, temp, 0);
		cpu_modify_flag_half_carry(sys, *(uint8_t*)regmem, temp);
		cpu_modify_flag_zero(sys, temp);
		cpu_modify_flag_parity(sys, temp);
		cpu_modify_flag_sign(sys, temp, 0);
		cpu_modify_flag_overflow(sys, *(uint8_t*)regmem, *(uint8_t*)reg, temp, 0);

		break;
	}
	case CMP_RM16_R16: // 39 mm
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

		uint16_t temp = *(uint16_t*)regmem - *(uint16_t*)reg;

		cpu_modify_flag_carry(sys, *(uint16_t*)regmem, temp, 1);
		cpu_modify_flag_half_carry(sys, *(uint16_t*)regmem, temp);
		cpu_modify_flag_zero(sys, temp);
		cpu_modify_flag_parity(sys, temp);
		cpu_modify_flag_sign(sys, temp, 1);
		cpu_modify_flag_overflow(sys, *(uint8_t*)regmem, *(uint8_t*)reg, temp, 1);

		break;
	}
	case CMP_R8_RM8: // 3A mm
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

		uint8_t temp = *(uint8_t*)reg - *(uint8_t*)regmem;

		cpu_modify_flag_carry(sys, *(uint8_t*)reg, temp, 0);
		cpu_modify_flag_half_carry(sys, *(uint8_t*)reg, temp);
		cpu_modify_flag_zero(sys, temp);
		cpu_modify_flag_parity(sys, temp);
		cpu_modify_flag_sign(sys, temp, 0);
		cpu_modify_flag_overflow(sys, *(uint8_t*)reg, *(uint8_t*)regmem, temp, 0);

		break;
	}
	case CMP_R16_RM16: // 3B mm
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

		uint16_t temp = *(uint16_t*)reg - *(uint16_t*)regmem;

		cpu_modify_flag_carry(sys, *(uint16_t*)reg, temp, 0);
		cpu_modify_flag_half_carry(sys, *(uint16_t*)reg, temp);
		cpu_modify_flag_zero(sys, temp);
		cpu_modify_flag_parity(sys, temp);
		cpu_modify_flag_sign(sys, temp, 0);
		cpu_modify_flag_overflow(sys, *(uint8_t*)reg, *(uint8_t*)regmem, temp, 1);

		break;
	}
	// 0x48 + i
	case DEC_R16:
	case DEC_R16 + 1:
	case DEC_R16 + 2:
	case DEC_R16 + 3:
	case DEC_R16 + 4:
	case DEC_R16 + 5:
	case DEC_R16 + 6:
	case DEC_R16 + 7:
	{
		int reg_code = opcode - DEC_R16;

		reg = reg16_index(&sys->cpu, reg_code);

		uint16_t old_val = (*(uint16_t*)reg);

		(*(uint16_t*)reg)--;

		cpu_modify_flag_half_carry(sys, old_val, (*(uint16_t*)reg));
		cpu_modify_flag_zero(sys, (*(uint16_t*)reg));
		cpu_modify_flag_parity(sys, (*(uint16_t*)reg));
		cpu_modify_flag_sign(sys, (*(uint16_t*)reg), 1);

		ip_increase = 1;

		break;
	}
	// 0x40 + i
	case INC_R16:
	case INC_R16 + 1:
	case INC_R16 + 2:
	case INC_R16 + 3:
	case INC_R16 + 4:
	case INC_R16 + 5:
	case INC_R16 + 6:
	case INC_R16 + 7:
	{
		int reg_code = opcode - INC_R16;

		reg = reg16_index(&sys->cpu, reg_code);

		uint16_t old_val = (*(uint16_t*)reg);

		(*(uint16_t*)reg)++;

		cpu_modify_flag_half_carry(sys, old_val, (*(uint16_t*)reg));
		cpu_modify_flag_zero(sys, (*(uint16_t*)reg));
		cpu_modify_flag_parity(sys, (*(uint16_t*)reg));
		cpu_modify_flag_sign(sys, (*(uint16_t*)reg), 1);

		ip_increase = 1;
		break;
	}
	case INT_IMM8: // CD ii
	{
		uint8_t interrupt = sys->memory[cur_inst + 1];

		// Look up in interrupt vector table. Example: int 0x10, 0x10 * 4 = 0x40, get offset at 0x40 and segment at 0x42, jump there 

		uint16_t* interrupt_offset = &sys->memory[seg_mem(0, interrupt * 4)];
		uint16_t* interrupt_segment = &sys->memory[seg_mem(0, interrupt * 4) + 2];

		sys->cpu.sp.whole -= 2;

		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*stack = sys->cpu.flag.whole;

		sys->cpu.sp.whole -= 2;

		stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*stack = sys->cpu.cs.whole;

		sys->cpu.sp.whole -= 2;

		stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*stack = sys->cpu.ip.whole + 2; // +2 for next instruction

		sys->cpu.ip.whole = *interrupt_offset;
		sys->cpu.cs.whole = *interrupt_segment;

			break;
		}

		ip_increase = 2;
		break;
	}
	case JA_REL8: // 77 ii
	{
		if ((sys->cpu.flag.whole & FLAG_CARRY) == 0 && (sys->cpu.flag.whole & FLAG_ZERO) == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}
		
		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JAE_REL8: // 73 ii // Same as JNB_REL8, JNC_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_CARRY) == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}
		break;
	}
	case JB_REL8: // 72 ii // Same as JC_REL8, JNAE_REL8
	{
		if (sys->cpu.flag.whole & FLAG_CARRY)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JBE_REL8: // 76 ii // Same as JNA_REL8
	{
		if (sys->cpu.flag.whole & FLAG_CARRY || sys->cpu.flag.whole & FLAG_ZERO)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}
		break;
	}
	case JCXZ_REL8: // E3 ii
	{
		if (sys->cpu.cx.whole == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JE_REL8: // 74 ii // Same as JZ_REL8
	{
		if (sys->cpu.flag.whole & FLAG_ZERO)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JG_REL8: // 7f ii // Same as JNLE_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_ZERO) == 0 && (sys->cpu.flag.whole & FLAG_SIGN) == (sys->cpu.flag.whole & FLAG_OVERFLOW))
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JGE_REL8: // 7D ii // Same as JNL_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_SIGN) == (sys->cpu.flag.whole & FLAG_OVERFLOW))
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JL_REL8: // 7c ii // Same as JNGE_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_SIGN) != (sys->cpu.flag.whole & FLAG_OVERFLOW))
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JLE_REL8: // 7E ii // Same as JNG_REL8
	{
		if (sys->cpu.flag.whole & FLAG_ZERO || (sys->cpu.flag.whole & FLAG_SIGN) != (sys->cpu.flag.whole & FLAG_OVERFLOW))
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

	}
	case JNE_REL8: // 75 ii // Same as JNZ_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_ZERO) == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JNO_REL8: // 71 ii
	{
		if ((sys->cpu.flag.whole & FLAG_OVERFLOW) == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JNP_REL8: // 7b ii // Same as JPO_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_PARITY) == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JNS_REL8: // 79 ii
	{
		if ((sys->cpu.flag.whole & FLAG_SIGN) == 0)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JO_REL8: // 77 ii
	{
		if (sys->cpu.flag.whole & FLAG_OVERFLOW)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JP_REL8: // 7A ii // Same as JPE_REL8
	{
		if (sys->cpu.flag.whole & FLAG_OVERFLOW)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JS_REL8: // 78 ii
	{
		if (sys->cpu.flag.whole & FLAG_SIGN)
		{
			ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		}

		else
		{
			ip_increase += 2;
		}

		break;
	}
	case JMP_REL8: // EB ii
	{
		ip_increase = (int8_t)sys->memory[cur_inst + 1] + 2;
		break;
	}
	case JMP_REL16: // E9 ii ii
	{
		int16_t* jmp_value = &sys->memory[cur_inst + 1];

		ip_increase = *jmp_value + 3;
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
	case MOV_RM8_R8: // 88 mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
		*(uint8_t*)regmem = *(uint8_t*)reg;

		break;
	}
	case MOV_RM16_R16: // 89 mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
		*(uint16_t*)regmem = *(uint16_t*)reg;

		break;
	}
	case MOV_R8_RM8: // 8A mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

		*(uint8_t*)reg = *(uint8_t*)regmem;

		break;
	}
	case MOV_R16_RM16: // 8B mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

		*(uint16_t*)reg = *(uint16_t*)regmem;

		break;
	}
	case MOV_RM16_SREG: // 8C mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 1);

		*(uint16_t*)regmem = *(uint16_t*)reg;

		break;
	}
	case MOV_SREG_RM16: // 8E mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 1);

		*(uint16_t*)reg = *(uint16_t*)regmem;

		break;
	}
	case MOV_AL_MOFFS8: // A0 dd dd
	{
		uint16_t* moffs8 = &sys->memory[cur_inst + 1];

		sys->cpu.ax.low = sys->memory[seg_mem(data_seg->whole, *moffs8)];

		ip_increase = 3;
		break;
	}
	case MOV_AX_MOFFS16: // A1 dd dd
	{
		uint16_t* moffs16 = &sys->memory[cur_inst + 1];
		uint16_t* moffs16_real = &sys->memory[seg_mem(data_seg->whole, *moffs16)];

		sys->cpu.ax.whole = *moffs16_real;

		ip_increase = 3;
		break;
	}
	case MOV_MOFFS8_AL: // A2 dd dd
	{
		uint16_t* moffs8 = &sys->memory[cur_inst + 1];

		sys->memory[seg_mem(data_seg->whole, *moffs8)] = sys->cpu.ax.low;

		ip_increase = 3;
		break;
	}
	case MOV_MOFFS16_AX: // A2 dd dd
	{
		uint16_t* moffs16 = &sys->memory[cur_inst + 1];
		uint16_t* moffs16_real = &sys->memory[seg_mem(data_seg->whole, *moffs16)];

		*moffs16_real = sys->cpu.ax.whole;

		ip_increase = 3;
		break;
	}
	// B0 ii
	case MOV_R8_IMM8: 
	case MOV_R8_IMM8 + 1:
	case MOV_R8_IMM8 + 2:
	case MOV_R8_IMM8 + 3:
	case MOV_R8_IMM8 + 4:
	case MOV_R8_IMM8 + 5:
	case MOV_R8_IMM8 + 6:
	case MOV_R8_IMM8 + 7:
	{
		int reg_code = opcode - MOV_R8_IMM8;

		reg = reg8_index(&sys->cpu, reg_code);

		*(uint8_t*)reg = sys->memory[cur_inst + 1];

		ip_increase = 2;
		break;
	}
	case MOV_R16_IMM16: // B8+x ii ii
	case MOV_R16_IMM16 + 1:
	case MOV_R16_IMM16 + 2:
	case MOV_R16_IMM16 + 3:
	case MOV_R16_IMM16 + 4:
	case MOV_R16_IMM16 + 5:
	case MOV_R16_IMM16 + 6:
	case MOV_R16_IMM16 + 7:
	{
		int reg_code = opcode - MOV_R16_IMM16;

		reg = reg16_index(&sys->cpu, reg_code);

		uint16_t* imm16 = &sys->memory[cur_inst + 1];
		*(uint16_t*)reg = *imm16;

		ip_increase = 3;

		break;
	}
	case MOV_RM8_IMM8: // C6 mm dd dd ii
	{
		uint8_t imm8 = 0;

		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm8, 0, 0, 0);

		*(uint8_t*)regmem = imm8;
		
		break;
	}
	case MOV_RM16_IMM16: // C7 mm dd dd ii ii
	{
		uint16_t imm16 = 0;

		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm16, 1, 1, 0);

		*(uint16_t*)regmem = imm16;		

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

		reg = reg16_index(&sys->cpu, reg_code);

		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*stack = *(uint16_t*)reg;

		ip_increase = 1;
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

		ip_increase = 1;
		break;
	}
	case POP_RM16: // 8F mm dd dd
	{
		ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*(uint16_t*)regmem = *stack;

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

		reg = reg16_index(&sys->cpu, reg_code);

		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		*(uint16_t*)reg = *stack;

		sys->cpu.sp.whole += 2;
		ip_increase = 1;
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
		ip_increase = 1;
		break;
	}
	case RET_FAR: // CB
	{
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		sys->cpu.ip.whole = *stack;

		sys->cpu.sp.whole += 2;

		sys->cpu.cs.whole = *stack;

		sys->cpu.sp.whole += 2;

		break;
	}
	case RET_NEAR: // C3
	{
		uint16_t* stack = &sys->memory[seg_mem(sys->cpu.ss.whole, sys->cpu.sp.whole)];

		sys->cpu.ip.whole = *stack;

		sys->cpu.sp.whole += 2;

		break;
	}
	default:
		sys->cpu.ip.whole++;
		return;
	}

	sys->cpu.ip.whole += ip_increase;
}