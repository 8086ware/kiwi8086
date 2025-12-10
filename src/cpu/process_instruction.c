#include "system.h"
#include "cpu/instructions.h"
#include "memory.h"
#include "cpu/opcode_desc.h"

void cpu_get_instruction(Sys8086* sys, Instruction* instruction)
{
	_Bool prefix_instruction_done = 0;

	instruction->segment = sys->cpu.cs.whole;
	instruction->offset = sys->cpu.ip.whole;
	instruction->data_seg = NULL;

	while (!prefix_instruction_done)
	{
		enum CPU_Opcode cur_byte = read_address8(sys, seg_mem(instruction->segment, instruction->offset + instruction->length), 0);

		switch (cur_byte)
		{
		case PREFIX_ES:
		case PREFIX_CS:
		case PREFIX_SS:
		case PREFIX_DS:
		{
			instruction->length++;
			instruction->data_seg = segment_reg_index(&sys->cpu, (cur_byte >> 3) & 0x3);
			break;
		}
		case PREFIX_REP_OR_REPE:
		case PREFIX_REPNE:
		{
			instruction->rep = cur_byte; // repetition
			instruction->length++;
			break;
		}
		case 0xf0: // nothing
			instruction->length++;
			break;
		default:
		{
			prefix_instruction_done = 1;
		}
		}
	}

	//http://forthworks.com:8800/temp/opcodes.html

	enum CPU_Opcode opcode = read_address8(sys, seg_mem(instruction->segment, instruction->offset + instruction->length), 0);

	instruction->operation = opcode;

	instruction->length++;

	instruction->width = opcode_desc[opcode] & 0x1;

	instruction->data1_width = opcode_desc[opcode] & 0x100;
	instruction->data2_width = opcode_desc[opcode] & 0x200;

	if (instruction->width) // width
	{
		instruction->reg = &sys->cpu.ax.whole;
	}

	else
	{
		instruction->reg = &sys->cpu.ax.low;
	}

	if (opcode_desc[opcode] & 0x4) // modrm byte?
	{
		calc_modrm_byte(sys, instruction, seg_mem(instruction->segment, instruction->offset + instruction->length), opcode_desc[opcode] & 0x80);
	}

	if (opcode_desc[opcode] & 0x8 || ((opcode == GROUP_OPCODE_F7 || opcode == GROUP_OPCODE_F6) && (instruction->reg == &sys->cpu.ax.whole || instruction->reg == &sys->cpu.ax.low))) // data 1.
	{
		if (instruction->data1_width == 0)
		{
			instruction->data1[0] = read_address8(sys, seg_mem(instruction->segment, instruction->offset + instruction->length), 0);
			instruction->length++;
		}

		else
		{
			*(uint16_t*)(instruction->data1) = read_address16(sys, seg_mem(instruction->segment, instruction->offset + instruction->length), 0);
			instruction->length += 2;
		}
	}

	if (opcode_desc[opcode] & 0x10) // data 2.
	{
		if (instruction->data2_width == 0)
		{
			instruction->data2[0] = read_address8(sys, seg_mem(instruction->segment, instruction->offset + instruction->length), 0);
			instruction->length++;
		}

		else
		{
			*(uint16_t*)(instruction->data2) = read_address16(sys, seg_mem(instruction->segment, instruction->offset + instruction->length), 0);
			instruction->length += 2;
		}
	}

	if (opcode_desc[opcode] & 0x20) // is segment in the opcode?
	{
		instruction->reg = segment_reg_index(&sys->cpu, (opcode >> 3) & 0x3);
	}

	else if (opcode_desc[opcode] & 0x40) // is register in opcode last 3 bits?
	{
		if (instruction->width)
		{
			instruction->reg = reg16_index(&sys->cpu, opcode & 0x7);
		}

		else
		{
			instruction->reg = reg8_index(&sys->cpu, opcode & 0x7);
		}
	}

	if (opcode_desc[opcode] & 0x2) // reg/regmem direction
	{
		instruction->regmem_to_reg = 1;
	}

	if (instruction->data_seg == NULL)
	{
		instruction->data_seg = &sys->cpu.ds.whole;

	}

	return instruction;
}

int cpu_exec_instruction(Sys8086* sys, Instruction* instruction)
{
	uint8_t group_opcode_instruction = 0;

	for (int i = 0; i < 8; i++)
	{
		if (instruction->width)
		{
			if (instruction->reg == reg16_index(&sys->cpu, i))
			{
				group_opcode_instruction = i;
				break;
			}
		}
		
		else
		{
			if (instruction->reg == reg8_index(&sys->cpu, i))
			{
				group_opcode_instruction = i;
				break;
			}
		}
	}

	switch (instruction->operation) // actual opcode
	{
	case GROUP_OPCODE_80:
	{
		switch (group_opcode_instruction)
		{
		case ADD_RM8_IMM8:
		{
			add8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case ADC_RM8_IMM8:
		{
			add8(sys, instruction->regmem, instruction->data1[0] + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			break;
		}
		case SBB_RM8_IMM8:
		{
			sub8(sys, instruction->regmem, instruction->data1[0] + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			break;
		}
		case AND_RM8_IMM8:
		{
			and8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case CMP_RM8_IMM8: // 80 mm ii
		{
			cmp8(sys, *(uint8_t*)instruction->regmem, instruction->data1[0]);
			break;
		}
		case SUB_RM8_IMM8:
		{
			sub8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case OR_RM8_IMM8:
		{
			or8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case XOR_RM8_IMM8:
		{
			xor8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);

			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_81:
	{
		switch (group_opcode_instruction)
		{
		case ADD_RM16_IMM16:
		{
			add16(sys, instruction->regmem, *(uint16_t*)instruction->data1);
			break;
		}
		case ADC_RM16_IMM16:
		{
			add16(sys, instruction->regmem, *(uint16_t*)instruction->data1 + (uint16_t)(sys->cpu.flag.whole & FLAG_CARRY));
			break;
		}
		case SBB_RM16_IMM16:
		{
			sub16(sys, instruction->regmem, *(uint16_t*)instruction->data1 + (uint16_t)(sys->cpu.flag.whole & FLAG_CARRY));
			break;
		}
		case AND_RM16_IMM16:
		{
			and16(sys, instruction->regmem, *(uint16_t*)instruction->data1);
			break;
		}
		case CMP_RM16_IMM16: // 83 mm ii
		{
			cmp16(sys, *(uint16_t*)instruction->regmem, *(uint16_t*)instruction->data1);
			break;
		}
		case SUB_RM16_IMM16:
		{
			sub16(sys, instruction->regmem, *(uint16_t*)instruction->data1);
			break;
		}
		case OR_RM16_IMM16:
		{
			or16(sys, instruction->regmem, *(uint16_t*)instruction->data1);
			break;
		}
		case XOR_RM16_IMM16:
		{
			xor16(sys, instruction->regmem, *(uint16_t*)instruction->data1);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			
			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_83:
	{
		switch (group_opcode_instruction)
		{
		case ADD_RM16_IMM8:
		{
			add16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case ADC_RM16_IMM8:
		{
			add16(sys, instruction->regmem, instruction->data1[0] + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			break;
		}
		case SBB_RM16_IMM8:
		{
			sub16(sys, instruction->regmem, instruction->data1[0] + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			break;
		}
		case AND_RM16_IMM8:
		{
			and16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case CMP_RM16_IMM8: // 83 mm ii
		{
			cmp16(sys, *(uint16_t*)instruction->regmem, instruction->data1[0]);
			break;
		}
		case SUB_RM16_IMM8:
		{
			sub16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case OR_RM16_IMM8:
		{
			or16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case XOR_RM16_IMM8:
		{
			xor16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			
			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_C0:
	{
		switch (group_opcode_instruction)
		{
		case ROL_RM8_IMM8:
		{
			rol8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case ROR_RM8_IMM8:
		{
			ror8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case RCL_RM8_IMM8:
		{
			rcl8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case RCR_RM8_IMM8:
		{
			rcr8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case SAL_RM8_IMM8: // D0 mm
		{
			sal8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case SAR_RM8_IMM8: // D0 mm
		{
			sar8(sys, instruction->regmem, instruction->data1[0]);
		break;
	}
		case SHR_RM8_IMM8: // D0 mm
		{
			shr8(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);

			break;
		}
		}

		break;
	}
	case GROUP_OPCODE_C1:
	{
		switch (group_opcode_instruction)
		{
		case ROL_RM16_IMM8:
		{
			rol16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case ROR_RM16_IMM8:
		{
			ror16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case RCL_RM16_IMM8:
		{
			rcl16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case RCR_RM16_IMM8:
		{
			rcr16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case SAL_RM16_IMM8: // D0 mm
		{
			sal16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case SAR_RM16_IMM8: // D0 mm
		{
			sar16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		case SHR_RM16_IMM8: // D0 mm
		{
			shr16(sys, instruction->regmem, instruction->data1[0]);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);

			break;
		}
		}

		break;
	}
	case GROUP_OPCODE_D0:
	{
		switch (group_opcode_instruction)
		{
		case ROL_RM8_1:
		{
			rol8(sys, instruction->regmem, 1);
			break;
		}
		case ROR_RM8_1:
		{
			ror8(sys, instruction->regmem, 1);
			break;
		}
		case RCL_RM8_1:
		{
			rcl8(sys, instruction->regmem, 1);
			break;
		}
		case RCR_RM8_1:
		{
			rcr8(sys, instruction->regmem, 1);
			break;
		}
		case SAL_RM8_1: // D0 mm
		{
			sal8(sys, instruction->regmem, 1);
			break;
		}
		case SAR_RM8_1: // D0 mm
		{
			sar8(sys, instruction->regmem, 1);
			break;
		}
		case SHR_RM8_1: // D0 mm
		{
			shr8(sys, instruction->regmem, 1);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			
			break;
		}
		}

		break;
	}
	case GROUP_OPCODE_D1:
	{
		switch (group_opcode_instruction)
		{
		case ROL_RM16_1:
		{
			rol16(sys, instruction->regmem, 1);
			break;
		}
		case ROR_RM16_1:
		{
			ror16(sys, instruction->regmem, 1);
			break;
		}
		case RCL_RM16_1:
		{
			rcl16(sys, instruction->regmem, 1);
			break;
		}
		case RCR_RM16_1:
		{
			rcr16(sys, instruction->regmem, 1);
			break;
		}
		case SAL_RM16_1: // D0 mm
		{
			sal16(sys, instruction->regmem, 1);
			break;
		}
		case SAR_RM16_1: // D0 mm
		{
			sar16(sys, instruction->regmem, 1);
			break;
		}
		case SHR_RM16_1: // D0 mm
		{
			shr16(sys, instruction->regmem, 1);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			
			break;
		}
		}

		break;
	}
	case GROUP_OPCODE_D2:
	{
		switch (group_opcode_instruction)
		{
		case ROL_RM8_CL:
		{
			rol8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case ROR_RM8_CL:
		{
			ror8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case RCL_RM8_CL:
		{
			rcl8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case RCR_RM8_CL:
		{
			rcr8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case SAL_RM8_CL: // D0 mm
		{
			sal8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case SAR_RM8_CL: // D0 mm
		{
			sar8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case SHR_RM8_CL: // D0 mm
		{
			shr8(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			
			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_D3:
	{
		switch (group_opcode_instruction)
		{
		case ROL_RM16_CL:
		{
			rol16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case ROR_RM16_CL:
		{
			ror16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case RCL_RM16_CL:
		{
			rcl16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case RCR_RM16_CL:
		{
			rcr16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case SAL_RM16_CL: // D0 mm
		{
			sal16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case SAR_RM16_CL: // D0 mm
		{
			sar16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		case SHR_RM16_CL: // D0 mm
		{
			shr16(sys, instruction->regmem, sys->cpu.cx.low);
			break;
		}
		default:
		{	
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_F6:
	{
		switch (group_opcode_instruction)
		{
		case TEST_RM8_IMM8:
		{
			uint8_t temp = *(uint8_t*)instruction->regmem;
			and8(sys, &temp, instruction->data1[0]);
			break;
		}
		case MUL_RM8: // F6 mm
		{
			mul8(sys, *(uint8_t*)instruction->regmem);
			break;
		}
		case NEG_RM8:
		{
			neg8(sys, instruction->regmem);
			break;
		}
		case NOT_RM8:
		{
			*(uint8_t*)instruction->regmem = ~(*(uint8_t*)instruction->regmem);
			break;
		}
		case DIV_RM8:
		{
			uint16_t to_divide = sys->cpu.ax.whole;
			sys->cpu.ax.low = to_divide / *(uint8_t*)instruction->regmem;
			sys->cpu.ax.high = to_divide % *(uint8_t*)instruction->regmem;
			break;
		}
		case IDIV_RM8:
		{
			int16_t to_divide = sys->cpu.ax.whole;
			sys->cpu.ax.low = to_divide / *(int8_t*)instruction->regmem;
			sys->cpu.ax.high = to_divide % *(int8_t*)instruction->regmem;
			break;
		}
		default:
		{	
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			break;
		}
		}
		break;
	}
	case GROUP_OPCODE_F7:
	{
		switch (group_opcode_instruction)
		{
		case TEST_RM16_IMM16:
		{
			uint16_t temp = *(uint16_t*)instruction->regmem;
			and16(sys, &temp, *(uint16_t*)instruction->data1);
			break;
		}
		case MUL_RM16: // F7 mm
		{
			mul16(sys, *(uint16_t*)instruction->regmem);
			break;
		}
		case NEG_RM16:
		{
			neg16(sys, instruction->regmem);
			break;
		}
		case NOT_RM16:
		{
			*(uint16_t*)instruction->regmem = ~(*(uint16_t*)instruction->regmem);
			break;
		}
		case DIV_RM16:
		{
			uint32_t value_to_divide = sys->cpu.dx.whole << 16;
			value_to_divide |= sys->cpu.ax.whole;
			sys->cpu.ax.whole = value_to_divide / *(uint16_t*)instruction->regmem;
			sys->cpu.dx.whole = value_to_divide % *(uint16_t*)instruction->regmem;
			break;
		}
		case IDIV_RM16:
		{
			int32_t value_to_divide = sys->cpu.dx.whole << 16;
			value_to_divide |= sys->cpu.ax.whole;
			sys->cpu.ax.whole = value_to_divide / *(int16_t*)instruction->regmem;
			sys->cpu.dx.whole = value_to_divide % *(int16_t*)instruction->regmem;
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
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
			dec8(sys, instruction->regmem);
			break;
		}
		case INC_RM8: // FE mm dd dd
		{
			inc8(sys, instruction->regmem);
			break;
		}
		default:
		{	
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
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
			dec16(sys, instruction->regmem);
			break;
		}
		// 0x2
		case CALL_RM16:
		{
			push(sys, sys->cpu.ip.whole + instruction->length);
			jmp(sys, sys->cpu.cs.whole, *(uint16_t*)instruction->regmem);
			return 1;
		}
		// 0x3
		case CALL_M16_16: // FF mm
		{
			push(sys, sys->cpu.cs.whole);
			push(sys, sys->cpu.ip.whole + instruction->length);

			uint16_t segment = read_address16(sys, *(uint16_t*)instruction->regmem + 2, 0);
			uint16_t offset = read_address16(sys, *(uint16_t*)instruction->regmem, 0);

			jmp(sys, segment, offset);
			return 1;
		}
		// 0x4
		case JMP_RM16: // FF mm dd dd
		{
			jmp(sys, sys->cpu.cs.whole, *(uint16_t*)instruction->regmem);
			return 1;
		}
		case JMP_M16_16:
		{
			uint16_t segment = read_address16(sys, *(uint16_t*)instruction->regmem + 2, 0);
			uint16_t offset = read_address16(sys, *(uint16_t*)instruction->regmem, 0);

			jmp(sys, segment, offset);
			return 1;
		}
		// 0x6
		case PUSH_RM16: // FF mm dd dd
		{
			push(sys, *(uint16_t*)instruction->regmem);
			break;
		}
		// 0x0
		case INC_RM16: // FF mm dd dd
		{
			inc16(sys, instruction->regmem);
			break;
		}
		default:
		{
			printf("Unknown Opcode %x /%d\n", instruction->operation, group_opcode_instruction);
			break;
		}
		}

		break;
	}
	case AAD:
	{
		uint8_t tempal = sys->cpu.ax.low;
		uint8_t tempah = sys->cpu.ax.high;

		sys->cpu.ax.low = (tempal + (tempah * instruction->data1[0])) & 0xff;
		sys->cpu.ax.high = 0;

		cpu_modify_flag_sign(&sys->cpu, sys->cpu.ax.low, 0);
		cpu_modify_flag_zero(&sys->cpu, sys->cpu.ax.low);
		cpu_modify_flag_parity(&sys->cpu, sys->cpu.ax.low);
		break;
	}
	case AAM:
	{
		uint8_t temp = sys->cpu.ax.low;

		sys->cpu.ax.high = temp / instruction->data1[0];
		sys->cpu.ax.low = temp % instruction->data1[0];

		cpu_modify_flag_sign(&sys->cpu, sys->cpu.ax.low, 0);
		cpu_modify_flag_zero(&sys->cpu, sys->cpu.ax.low);
		cpu_modify_flag_parity(&sys->cpu, sys->cpu.ax.low);
		break;
	}
	case AAS:
	{
		if ((sys->cpu.ax.low & 0xf) > 9 || sys->cpu.flag.whole & FLAG_HALF_CARRY)
		{
			sys->cpu.ax.whole -= 6;
			sys->cpu.ax.high -= 1;
			sys->cpu.flag.whole |= FLAG_HALF_CARRY;
			sys->cpu.flag.whole |= FLAG_CARRY;
			sys->cpu.ax.low &= 0xf;
		}

		else
		{
			sys->cpu.flag.whole &= ~FLAG_HALF_CARRY;
			sys->cpu.flag.whole &= ~FLAG_CARRY;
			sys->cpu.ax.low &= 0xf;
		}
		break;
	}
	case ADC_AL_IMM8: // 04 ii
		add8(sys, instruction->reg, instruction->data1[0] + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
		break;
	case ADC_AX_IMM16: // 05 ii ii
		add16(sys, instruction->reg, *(uint16_t*)instruction->data1 + sys->cpu.flag.whole & FLAG_CARRY);
		break;
	case ADC_RM8_R8:
	case ADC_RM16_R16:
	case ADC_R8_RM8:
	case ADC_R16_RM16:
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				add16(sys, instruction->reg, *(uint16_t*)instruction->regmem + sys->cpu.flag.whole & FLAG_CARRY);
			}

			else
			{
				add8(sys, instruction->reg, *(uint8_t*)instruction->regmem + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			}
		}

		else
		{
			if (instruction->width)
			{
				add16(sys, instruction->regmem, *(uint16_t*)instruction->reg + sys->cpu.flag.whole & FLAG_CARRY);
			}

			else
			{
				add8(sys, instruction->regmem, *(uint8_t*)instruction->reg + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			}
		}
		break;
	}
	case ADD_AL_IMM8: // 04 ii
		add8(sys, instruction->reg, instruction->data1[0]);
		break;
	case ADD_AX_IMM16: // 05 ii ii
		add16(sys, instruction->reg, *(uint16_t*)instruction->data1);
		break;
	case ADD_RM8_R8: // 00 mm
	case ADD_RM16_R16: // 01 mm		
	case ADD_R8_RM8: // 02 mm
	case ADD_R16_RM16: // 03 mm
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				add16(sys, instruction->reg, *(uint16_t*)instruction->regmem);
			}

			else
			{
				add8(sys, instruction->reg, *(uint8_t*)instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				add16(sys, instruction->regmem, *(uint16_t*)instruction->reg);
			}

			else
			{
				add8(sys, instruction->regmem, *(uint8_t*)instruction->reg);
			}
		}
		break;
	}
	case AND_AL_IMM8:
		and8(sys, instruction->reg, instruction->data1[0]);
		break;
	case AND_AX_IMM16:
		and16(sys, instruction->reg, *(uint16_t*)instruction->data1);
		break;
	case AND_RM8_R8:
	case AND_RM16_R16:
	case AND_R8_RM8:
	case AND_R16_RM16:
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				and16(sys, instruction->reg, *(uint16_t*)instruction->regmem);
			}

			else
			{
				and8(sys, instruction->reg, *(uint8_t*)instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				and16(sys, instruction->regmem, *(uint16_t*)instruction->reg);
			}

			else
			{
				and8(sys, instruction->regmem, *(uint8_t*)instruction->reg);
			}
		}
		break;
	}
	case CALL_PTR16_16: // 9A ii ii ii ii
	{
		push(sys, sys->cpu.cs.whole);
		push(sys, sys->cpu.ip.whole + instruction->length);

		jmp(sys, *(uint16_t*)instruction->data1, *(uint16_t*)instruction->data2);

		return 1;
	}
	case CALL_REL16: // E8 ii ii
	{
		push(sys, sys->cpu.ip.whole + instruction->length);

		int16_t call_value = *(uint16_t*)instruction->data1 + instruction->length;

		jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + call_value);

		return 1;
	}
	case CBW:
	{
		if ((int8_t)sys->cpu.ax.low < 0)
		{
			sys->cpu.ax.high = 0xFF;
		}

		else
		{
			sys->cpu.ax.high = 0;
		}
		break;
	}
	case CLC:
	{
		sys->cpu.flag.whole &= ~FLAG_CARRY;
		break;
	}
	case CLD:
	{
		sys->cpu.flag.whole &= ~FLAG_DIRECTION;
		break;
	}
	case CLI:
	{
		sys->cpu.flag.whole &= ~FLAG_INTERRUPT;
		break;
	}
	case CMC:
	{
		if (sys->cpu.flag.whole & FLAG_CARRY)
		{
			sys->cpu.flag.whole &= ~FLAG_CARRY;
		}

		else
		{
			sys->cpu.flag.whole |= FLAG_CARRY;
		}

		break;
	}
	case CMP_AL_IMM8: // 3C ii
		cmp8(sys, *(uint8_t*)instruction->reg, instruction->data1[0]);
		break;
	case CMP_AX_IMM16: // 3D ii ii
		cmp16(sys, *(uint16_t*)instruction->reg, *(uint16_t*)instruction->data1);
		break;
	case CMP_RM8_R8: // 38 mm
	case CMP_RM16_R16: // 39 mm
	case CMP_R8_RM8: // 3A mm
	case CMP_R16_RM16: // 3B mm
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				cmp16(sys, *(uint16_t*)instruction->reg, *(uint16_t*)instruction->regmem);
			}

			else
			{
				cmp8(sys, *(uint8_t*)instruction->reg, *(uint8_t*)instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				cmp16(sys, *(uint16_t*)instruction->regmem, *(uint16_t*)instruction->reg);
			}

			else
			{
				cmp8(sys, *(uint8_t*)instruction->regmem, *(uint8_t*)instruction->reg);
			}
		}
		break;
	}
	case CMPSB:
	{
		cmp8(sys, read_address8(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0), read_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), 0));

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole--;
			sys->cpu.di.whole--;
		}

		else
		{
			sys->cpu.si.whole++;
			sys->cpu.di.whole++;
		}

		sys->cpu.zero_flag_check = 1;
		break;
	}
	case CMPSW:
	{
		cmp16(sys, read_address16(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0), read_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), 0));

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole -= 2;
			sys->cpu.di.whole -= 2;
		}

		else
		{
			sys->cpu.si.whole += 2;
			sys->cpu.di.whole += 2;
		}

		sys->cpu.zero_flag_check = 1;
		break;
	}
	case CWD:
	{
		if ((int16_t)sys->cpu.ax.whole < 0)
		{
			sys->cpu.dx.whole = 0xFFFF;
		}

		else
		{
			sys->cpu.dx.whole = 0;
		}

		break;
	}
	case DAS:
	{
		uint8_t old_al = sys->cpu.ax.low;
		_Bool old_cf = sys->cpu.flag.whole & FLAG_CARRY;

		sys->cpu.flag.whole &= ~FLAG_CARRY;

		if ((sys->cpu.ax.low & 0xf) > 9 || sys->cpu.flag.whole & FLAG_HALF_CARRY)
		{
			sys->cpu.ax.low -= 6;

			if (old_cf)
			{
				sys->cpu.flag.whole |= FLAG_CARRY;
			}

			else
			{
				sys->cpu.flag.whole &= ~FLAG_CARRY;
			}

			sys->cpu.flag.whole |= FLAG_HALF_CARRY;
		}

		else
		{
			sys->cpu.flag.whole &= ~FLAG_HALF_CARRY;
		}

		if ((old_al > 0x99) || old_cf)
		{
			sys->cpu.ax.low -= 0x60;
			sys->cpu.flag.whole |= FLAG_CARRY;
		}
		break;
	}
	// 0x48 + i
	case DEC_AX:
	case DEC_CX:
	case DEC_DX:
	case DEC_BX:
	case DEC_SP:
	case DEC_BP:
	case DEC_SI:
	case DEC_DI:
	{
		dec16(sys, instruction->reg);
		break;
	}
	case HLT:
	{
		sys->cpu.halted = 1;
		break;
	}
	case IN_AL_IMM8:
	{
		sys->cpu.ax.low = read_address8(sys, instruction->data1[0], 1);
		break;
	}
	case IN_AX_IMM8:
	{
		sys->cpu.ax.whole = read_address16(sys, instruction->data1[0], 1);
		break;
	}
	case IN_AL_DX:
	{
		sys->cpu.ax.low = read_address8(sys, sys->cpu.dx.whole, 1);
		break;
	}
	case IN_AX_DX:
	{
		sys->cpu.ax.whole = read_address16(sys, sys->cpu.dx.whole, 1);
		break;
	}
	case INSB:
	{
		sys->cpu.ax.low = read_address8(sys, sys->cpu.dx.whole, 1);

		write_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), sys->cpu.ax.low, 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.di.whole--;
		}

		else
		{
			sys->cpu.di.whole++;
		}

		break;
	}
	case INSW:
	{
		sys->cpu.ax.whole = read_address16(sys, sys->cpu.dx.whole, 1);

		write_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), sys->cpu.ax.whole, 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.di.whole-=2;
		}

		else
		{
			sys->cpu.di.whole+=2;
		}

		break;
	}
	// 40 + i
	case INC_AX:
	case INC_CX:
	case INC_DX:
	case INC_BX:
	case INC_SP:
	case INC_BP:
	case INC_SI:
	case INC_DI:
	{
		inc16(sys, instruction->reg);
		break;
	}
	case INT_IMM8: // CD ii
	{
		uint8_t interrupt = instruction->data1[0];

		// Look up in interrupt vector table. Example: int 0x10, 0x10 * 4 = 0x40, get offset at 0x40 and segment at 0x42, jump there 

		uint16_t interrupt_offset = read_address16(sys, seg_mem(0, interrupt * 4), 0);
		uint16_t interrupt_segment = read_address16(sys, seg_mem(0, interrupt * 4) + 2, 0);

		push(sys, sys->cpu.flag.whole);
		push(sys, sys->cpu.cs.whole);
		push(sys, sys->cpu.ip.whole + instruction->length);

		jmp(sys, interrupt_segment, interrupt_offset);

		return 1;
	}
	case IRET: // CF
	{
		// pop ip, cs, and flags after interrupt

		pop(sys, &sys->cpu.ip.whole);
		pop(sys, &sys->cpu.cs.whole);
		pop(sys, &sys->cpu.flag.whole);

		return 1;
	}
	case JA_REL8: // 77 ii
	{
		if ((sys->cpu.flag.whole & FLAG_CARRY) == 0 && (sys->cpu.flag.whole & FLAG_ZERO) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JAE_REL8: // 73 ii // Same as JNB_REL8, JNC_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_CARRY) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JB_REL8: // 72 ii // Same as JC_REL8, JNAE_REL8
	{
		if (sys->cpu.flag.whole & FLAG_CARRY)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JBE_REL8: // 76 ii // Same as JNA_REL8
	{
		if (sys->cpu.flag.whole & FLAG_CARRY || sys->cpu.flag.whole & FLAG_ZERO)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JCXZ_REL8: // E3 ii
	{
		if (sys->cpu.cx.whole == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JE_REL8: // 74 ii // Same as JZ_REL8
	{
		if (sys->cpu.flag.whole & FLAG_ZERO)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JG_REL8: // 7f ii // Same as JNLE_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_ZERO) == 0 && (((sys->cpu.flag.whole & FLAG_SIGN) == 0 && (sys->cpu.flag.whole & FLAG_OVERFLOW) == 0) || ((sys->cpu.flag.whole & FLAG_SIGN) && (sys->cpu.flag.whole & FLAG_OVERFLOW))))
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JGE_REL8: // 7D ii // Same as JNL_REL8
	{
		if (((sys->cpu.flag.whole & FLAG_SIGN) == 0 && (sys->cpu.flag.whole & FLAG_OVERFLOW) == 0) || ((sys->cpu.flag.whole & FLAG_SIGN) && (sys->cpu.flag.whole & FLAG_OVERFLOW)))
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JL_REL8: // 7c ii // Same as JNGE_REL8
	{
		if (((sys->cpu.flag.whole & FLAG_SIGN) == 0 && sys->cpu.flag.whole & FLAG_OVERFLOW) || (sys->cpu.flag.whole & FLAG_SIGN && (sys->cpu.flag.whole & FLAG_OVERFLOW) == 0))
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JLE_REL8: // 7E ii // Same as JNG_REL8
	{
		if (sys->cpu.flag.whole & FLAG_ZERO || (((sys->cpu.flag.whole & FLAG_SIGN) == 0 && sys->cpu.flag.whole & FLAG_OVERFLOW) || (sys->cpu.flag.whole & FLAG_SIGN && (sys->cpu.flag.whole & FLAG_OVERFLOW) == 0)))
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JNE_REL8: // 75 ii // Same as JNZ_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_ZERO) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JNO_REL8: // 71 ii
	{
		if ((sys->cpu.flag.whole & FLAG_OVERFLOW) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JNP_REL8: // 7b ii // Same as JPO_REL8
	{
		if ((sys->cpu.flag.whole & FLAG_PARITY) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JNS_REL8: // 79 ii
	{
		if ((sys->cpu.flag.whole & FLAG_SIGN) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JO_REL8: // 77 ii
	{
		if (sys->cpu.flag.whole & FLAG_OVERFLOW)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JP_REL8: // 7A ii // Same as JPE_REL8
	{
		if (sys->cpu.flag.whole & FLAG_PARITY)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JS_REL8: // 78 ii
	{
		if (sys->cpu.flag.whole & FLAG_SIGN)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}
		break;
	}
	case JMP_REL8: // EB ii
	{
		jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
		return 1;
	}
	case JMP_REL16: // E9 ii ii
	{
		jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + *(int16_t*)instruction->data1 + instruction->length);
		return 1;
	}
	case JMP_PTR16_16: // EA ii ii ii ii
	{
		uint16_t new_ip = *(uint16_t*)instruction->data1;
		uint16_t new_cs = *(uint16_t*)instruction->data2;

		jmp(sys, new_cs, new_ip);
		return 1;
	}
	case LAHF:
	{
		sys->cpu.ax.high = sys->cpu.flag.whole & 0x00ff;
		break;
	}
	case LDS:
	{
		*(uint16_t*)instruction->reg = *(uint16_t*)instruction->regmem;
		(uint8_t*)instruction->regmem += 2;
		sys->cpu.ds.whole = *(uint16_t*)instruction->regmem;
		break;
	}
	case LEA:
	{
		*(uint16_t*)instruction->reg = (((uint8_t*)instruction->regmem - (instruction->data_seg->whole * 0x10)) - (uint8_t*)(&sys->memory));
		break;
	}
	case LES:
	{
		*(uint16_t*)instruction->reg = *(uint16_t*)instruction->regmem;
		(uint8_t*)instruction->regmem += 2;
		sys->cpu.es.whole = *(uint16_t*)instruction->regmem;
		break;
	}
	case LODSB:
	{
		sys->cpu.ax.low = read_address8(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole--;
		}

		else
		{
			sys->cpu.si.whole++;
		}

		
		break;
	}
	case LODSW:
	{
		sys->cpu.ax.whole = read_address16(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole -= 2;
		}

		else
		{
			sys->cpu.si.whole += 2;
		}

		break;
	}
	case LOOP_REL8:
	{
		sys->cpu.cx.whole--;

		if (sys->cpu.cx.whole != 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}

		break;
	}
	case LOOPE_REL8:
	{
		sys->cpu.cx.whole--;

		if (sys->cpu.cx.whole != 0 && sys->cpu.flag.whole & FLAG_ZERO)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}

		break;
	}
	case LOOPNE_REL8:
	{
		sys->cpu.cx.whole--;

		if (sys->cpu.cx.whole != 0 && (sys->cpu.flag.whole & FLAG_ZERO) == 0)
		{
			jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)instruction->data1[0] + instruction->length);
			return 1;
		}

		break;
	}
	case MOV_RM8_IMM8: // C6 mm dd dd ii
		mov8(sys, instruction->regmem, &instruction->data1[0]);
		break;
	case MOV_RM16_IMM16: // C7 mm dd dd ii ii
		mov16(sys, instruction->regmem, &instruction->data1);
		break;
	case MOV_RM8_R8: // 88 mm dd dd
	case MOV_RM16_R16: // 89 mm dd dd
	case MOV_R8_RM8: // 8A mm dd dd
	case MOV_R16_RM16: // 8B mm dd dd
	case MOV_RM16_SREG: // 8C mm dd dd
	case MOV_SREG_RM16: // 8E mm dd dd
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				mov16(sys, instruction->reg, instruction->regmem);
			}

			else
			{
				mov8(sys, instruction->reg, instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				mov16(sys, instruction->regmem, instruction->reg);
			}

			else
			{
				mov8(sys, instruction->regmem, instruction->reg);
			}
		}
		break;
	}
	// B0 ii
	case MOV_AL_IMM8:
	case MOV_CL_IMM8:
	case MOV_DL_IMM8:
	case MOV_BL_IMM8:
	case MOV_AH_IMM8:
	case MOV_CH_IMM8:
	case MOV_DH_IMM8:
	case MOV_BH_IMM8:
		mov8(sys, instruction->reg, &instruction->data1[0]);
		break;
	case MOV_AX_IMM16: // B8+x ii ii
	case MOV_CX_IMM16:
	case MOV_DX_IMM16:
	case MOV_BX_IMM16:
	case MOV_SP_IMM16:
	case MOV_BP_IMM16:
	case MOV_SI_IMM16:
	case MOV_DI_IMM16:
		mov16(sys, instruction->reg, &instruction->data1);
		break;
	case MOV_AL_MOFFS8: // A0 dd dd
	{
		mov8(sys, &sys->cpu.ax.low, &sys->memory[seg_mem(instruction->data_seg->whole, *(uint16_t*)instruction->data1)]);
		break;
	}
	case MOV_AX_MOFFS16: // A1 dd dd
	{
		mov16(sys, &sys->cpu.ax.whole, (&sys->memory[seg_mem(instruction->data_seg->whole, *(uint16_t*)instruction->data1)]));
		break;
	}
	case MOV_MOFFS8_AL: // A2 dd dd	
	{
		mov8(sys, &sys->memory[seg_mem(instruction->data_seg->whole, *(uint16_t*)instruction->data1)], &sys->cpu.ax.low);
		break;
	}
	case MOV_MOFFS16_AX: // A2 dd dd
	{
		mov16(sys, &sys->memory[seg_mem(instruction->data_seg->whole, *(uint16_t*)instruction->data1)], &sys->cpu.ax.whole);
		break;
	}
	case MOVSB:
	{
		write_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), read_address8(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0), 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole--;
			sys->cpu.di.whole--;
		}

		else
		{
			sys->cpu.si.whole++;
			sys->cpu.di.whole++;
		}
		
		break;
	}
	case MOVSW:
	{
		write_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), read_address16(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0), 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole -= 2;
			sys->cpu.di.whole -= 2;
		}

		else
		{
			sys->cpu.si.whole += 2;
			sys->cpu.di.whole += 2;
		}
	
		break;
	}
	case OR_AL_IMM8:
		or8(sys, instruction->reg, instruction->data1[0]);
		break;
	case OR_AX_IMM16:
		or16(sys, instruction->reg, *(uint16_t*)instruction->data1);
		break;
	case OR_RM8_R8:
	case OR_RM16_R16:
	case OR_R8_RM8:
	case OR_R16_RM16:
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				or16(sys, instruction->reg, *(uint16_t*)instruction->regmem);
			}

			else
			{
				or8(sys, instruction->reg, *(uint8_t*)instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				or16(sys, instruction->regmem, *(uint16_t*)instruction->reg);
			}

			else
			{
				or8(sys, instruction->regmem, *(uint8_t*)instruction->reg);
			}
		}
		break;
	}
	case OUT_IMM8_AL:
	{
		write_address8(sys, instruction->data1[0], sys->cpu.ax.low, 1);
		break;
	}
	case OUT_IMM8_AX:
	{
		write_address16(sys, instruction->data1[0], sys->cpu.ax.whole, 1);
		break;
	}
	case OUT_DX_AL:
	{
		write_address8(sys, sys->cpu.dx.whole, sys->cpu.ax.low, 1);
		break;
	}
	case OUT_DX_AX:
	{
		write_address16(sys, sys->cpu.dx.whole, sys->cpu.ax.whole, 1);
		break;
	}
	case OUTSB:
	{
		sys->cpu.ax.low = read_address8(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole--;
		}

		else
		{
			sys->cpu.si.whole++;
		}

		write_address8(sys, sys->cpu.dx.whole, sys->cpu.ax.low, 1);
		break;
	}
	case OUTSW:
	{
		sys->cpu.ax.whole = read_address16(sys, seg_mem(instruction->data_seg->whole, sys->cpu.si.whole), 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.si.whole-=2;
		}

		else
		{
			sys->cpu.si.whole+=2;
		}

		write_address16(sys, sys->cpu.dx.whole, sys->cpu.ax.whole, 1);
		break;
	}
	case PUSHA:
	{
		for (int i = 0; i < 8; i++)
		{
			push(sys, *reg16_index(&sys->cpu, i));
		}
		break;
	}
	case PUSH_AX: // 50 + i
	case PUSH_CX:
	case PUSH_DX:
	case PUSH_BX:
	case PUSH_SP:
	case PUSH_BP:
	case PUSH_SI:
	case PUSH_DI:
	case PUSH_ES: // es
	case PUSH_CS: // cs
	case PUSH_SS: // ss
	case PUSH_DS: // ds
	{
		push(sys, *(uint16_t*)instruction->reg);
		break;
	}
	case PUSHF:
	{
		push(sys, sys->cpu.flag.whole);
		break;
	}
	case PUSH_IMM8:
	{
		push(sys, instruction->data1[0]);
		break;
	}
	case PUSH_IMM16:
	{
		push(sys, *(uint16_t*)instruction->data1);
		break;
	}
	case POPA:
	{
		for (int i = 7; i >= 0; i--)
		{
			if (i == 4) // popping sp (into nothing)
			{
				int garbo; 
				pop(sys, &garbo);
			}

			else
			{
				pop(sys, reg16_index(&sys->cpu, i));
			}
		}
		break;
	}
	case POP_RM16: // 8F mm dd dd
	{
		pop(sys, instruction->regmem);
		break;
	}
	case POP_AX: // 58 + i
	case POP_CX:
	case POP_DX:
	case POP_BX:
	case POP_SP:
	case POP_BP:
	case POP_SI:
	case POP_DI:
	// 07
	case POP_ES: // es
	case POP_CS: // cs
	case POP_SS: // ss
	case POP_DS: // ds
	{
		pop(sys, instruction->reg);
		break;
	}
	case POPF:
	{
		pop(sys, &sys->cpu.flag.whole);
		break;
	}
	case RET_FAR:
	{
		pop(sys, &sys->cpu.ip.whole);
		pop(sys, &sys->cpu.cs.whole);
		return 1;
	}
	case RET_FAR_IMM16:
	{
		pop(sys, &sys->cpu.ip.whole);
		pop(sys, &sys->cpu.cs.whole);
		sys->cpu.sp.whole += *(uint16_t*)instruction->data1;
		return 1;
	}
	case RET_NEAR:
	{
		pop(sys, &sys->cpu.ip.whole);
		return 1;
	}
	case RET_NEAR_IMM16:
	{
		pop(sys, &sys->cpu.ip.whole);
		sys->cpu.sp.whole += *(uint16_t*)instruction->data1;
		return 1;
	}
	case SAHF:
	{
		sys->cpu.flag.whole &= ~0x00ff;
		sys->cpu.flag.whole |= sys->cpu.ax.high;
		break;
	}
	case SBB_AL_IMM8:
		sub8(sys, instruction->reg, instruction->data1[0] + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
		break;
	case SBB_AX_IMM16:
		sub16(sys, instruction->reg, *(uint16_t*)instruction->data1 + sys->cpu.flag.whole & FLAG_CARRY);
		break;
	case SBB_RM8_R8:
	case SBB_RM16_R16:
	case SBB_R8_RM8:
	case SBB_R16_RM16:
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				sub16(sys, instruction->reg, *(uint16_t*)instruction->regmem + sys->cpu.flag.whole & FLAG_CARRY);
			}

			else
			{
				sub8(sys, instruction->reg, *(uint8_t*)instruction->regmem + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			}
		}

		else
		{
			if (instruction->width)
			{
				sub16(sys, instruction->regmem, *(uint16_t*)instruction->reg + sys->cpu.flag.whole & FLAG_CARRY);
			}

			else
			{
				sub8(sys, instruction->regmem, *(uint8_t*)instruction->reg + (uint8_t)(sys->cpu.flag.whole & FLAG_CARRY));
			}
		}
		break;
	}
	case SCASB:
	{
		cmp8(sys, sys->cpu.ax.low, read_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), 0));

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.di.whole--;
		}

		else
		{
			sys->cpu.di.whole++;
		}

		sys->cpu.zero_flag_check = 1;
	
		break;
	}
	case SCASW:
	{
		cmp16(sys, sys->cpu.ax.whole, read_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), 0));

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.di.whole -= 2;
		}

		else
		{
			sys->cpu.di.whole += 2;
		}

		sys->cpu.zero_flag_check = 1;
		
		break;
	}
	case STC:
	{
		sys->cpu.flag.whole |= FLAG_CARRY;	
		break;
	}
	case STD:
	{
		sys->cpu.flag.whole |= FLAG_DIRECTION;
		break;
	}
	case STI:
	{
		sys->cpu.sti_enable = 1;
		break;
	}
	case STOSB:
	{
		write_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), sys->cpu.ax.low, 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.di.whole--;
		}

		else
		{
			sys->cpu.di.whole++;
		}
	
		break;
	}
	case STOSW:
	{
		write_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), sys->cpu.ax.whole, 0);

		if (sys->cpu.flag.whole & FLAG_DIRECTION)
		{
			sys->cpu.di.whole -= 2;
		}

		else
		{
			sys->cpu.di.whole += 2;
		}
		
		break;
	}
	case SUB_AL_IMM8:
		sub8(sys, instruction->reg, instruction->data1[0]);
		break;
	case SUB_AX_IMM16:
		sub16(sys, instruction->reg, *(uint16_t*)instruction->data1);
		break;
	case SUB_RM8_R8:
	case SUB_RM16_R16:
	case SUB_R8_RM8:
	case SUB_R16_RM16:
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				sub16(sys, instruction->reg, *(uint16_t*)instruction->regmem);
			}

			else
			{
				sub8(sys, instruction->reg, *(uint8_t*)instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				sub16(sys, instruction->regmem, *(uint16_t*)instruction->reg);
			}

			else
			{
				sub8(sys, instruction->regmem, *(uint8_t*)instruction->reg);
			}
		}
		break;
	}
	case TEST_AL_IMM8:
	{
		uint8_t temp = *(uint8_t*)instruction->reg;
		and8(sys, &temp, instruction->data1[0]);
		break;
	}
	case TEST_AX_IMM16:
	{
		uint16_t temp = *(uint16_t*)instruction->reg;
		and16(sys, &temp, *(uint16_t*)instruction->data1);
		break;
	}
	case TEST_RM8_R8:
	{
		uint8_t temp = *(uint8_t*)instruction->regmem;
		and8(sys, &temp, *(uint8_t*)instruction->reg);
		break;
	}
	case TEST_RM16_R16:
	{
		uint16_t temp = *(uint16_t*)instruction->regmem;
		and16(sys, &temp, *(uint16_t*)instruction->reg);
		break;
	}	
	case XCHG_RM8_R8:
	{
		uint8_t temp = *(uint8_t*)instruction->regmem;
		mov8(sys, instruction->regmem, instruction->reg);
		mov8(sys, instruction->reg, &temp);
		break;
	}
	case XCHG_RM16_R16:
	{
		uint16_t temp = *(uint16_t*)instruction->regmem;
		mov16(sys, instruction->regmem, instruction->reg);
		mov16(sys, instruction->reg, &temp);
		break;
	}
	case XCHG_AX_AX:
	case XCHG_AX_CX:
	case XCHG_AX_DX:
	case XCHG_AX_BX:
	case XCHG_AX_SP:
	case XCHG_AX_BP:
	case XCHG_AX_SI:
	case XCHG_AX_DI:
	{
		uint16_t temp = *(uint16_t*)instruction->reg;

		mov16(sys, instruction->reg, &sys->cpu.ax.whole);
		mov16(sys, &sys->cpu.ax.whole, &temp);
		break;
	}
	case XLAT:
	{
		uint16_t temp = (uint16_t)sys->cpu.ax.low;
		sys->cpu.ax.low = read_address8(sys, seg_mem(instruction->data_seg->whole, sys->cpu.bx.whole + temp), 0);
		break;
	}
	case XOR_AL_IMM8:
		xor8(sys, instruction->reg, instruction->data1[0]);
		break;
	case XOR_AX_IMM16:
		xor16(sys, instruction->reg, *(uint16_t*)instruction->data1);
		break;
	case XOR_RM8_R8:
	case XOR_RM16_R16:
	case XOR_R8_RM8:
	case XOR_R16_RM16:
	{
		if (instruction->regmem_to_reg)
		{
			if (instruction->width)
			{
				xor16(sys, instruction->reg, *(uint16_t*)instruction->regmem);
			}

			else
			{
				xor8(sys, instruction->reg, *(uint8_t*)instruction->regmem);
			}
		}

		else
		{
			if (instruction->width)
			{
				xor16(sys, instruction->regmem, *(uint16_t*)instruction->reg);
			}

			else
			{
				xor8(sys, instruction->regmem, *(uint8_t*)instruction->reg);
			}
		}
		break;
	}
	default:
		printf("[CPU] Unknown Opcode %x, %x:%x\n", instruction->operation, instruction->segment, instruction->offset);
		break;
	}			
	
	return 0;
}