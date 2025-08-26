#include "system.h"
#include "cpu/instructions.h"
#include "memory.h"
#include "pic.h"

void cpu_exec(Sys8086* sys)
{
	uint64_t now_ticks = SDL_GetTicksNS();

	int instructions_to_do = (now_ticks - sys->cpu.last_tick) / CPU_NANOSECONDS_PER_CYCLE;
	int irq_vector_offset = 0;

	sys->cpu.last_tick = now_ticks;

	for(int i = 0; i < instructions_to_do; i++)
	{
		sys->cpu.instructions++; // measuring ips
		if(sys->cpu.flag.whole & FLAG_INTERRUPT)
		{
			if (sys->pic_master.irr != 0 || sys->pic_slave.irr != 0)
			{
				// This if statement chain is based on the interrupt priority handling

				int irq_to_handle = 0;

				for(int i = 0; i < 8; i++)
				{
					if((sys->pic_master.irr >> i) & 0x1)
					{
						sys->pic_master.isr |= sys->pic_master.irr >> i;
						sys->pic_master.irr &= ~(1 << i);
						irq_to_handle = i;
						break;
					}
				}

				if(irq_to_handle == 2) // Slave pic (irq 2)
				{
					for(int i = 0; i < 8; i++)
					{
						if((sys->pic_slave.irr >> i) & 0x1)
						{
							sys->pic_slave.isr |= sys->pic_slave.irr >> i;
							sys->pic_slave.irr &= ~(1 << i);
							irq_to_handle = i + 8;
							break;
						}
					}
				}

				// Master pic vector offset
				else if (irq_to_handle <= 7)
				{
					irq_vector_offset = irq_to_handle * 4 + sys->pic_master.vector_offset;
				}

				// Slave pic vector offset
				else
				{
					irq_vector_offset = (irq_to_handle - 8) * 4 + sys->pic_slave.vector_offset;
				}

				push(sys, sys->cpu.flag.whole);
				push(sys, sys->cpu.cs.whole);
				push(sys, sys->cpu.ip.whole);

				sys->cpu.flag.whole &= ~FLAG_INTERRUPT;

				uint16_t interrupt_offset = read_address16(sys, seg_mem(0, irq_vector_offset), 0);
				uint16_t interrupt_segment = read_address16(sys, seg_mem(0, irq_vector_offset) + 2, 0);

				sys->cpu.ip.whole = interrupt_offset;
				sys->cpu.cs.whole = interrupt_segment;

				sys->cpu.halted = 0;
			}
		}

		// Fetch, Decode and execute instruction
		if (!sys->cpu.halted)
		{
			int ip_increase = 0;

			Register* data_seg = &sys->cpu.ds;
			Register* dest_seg = &sys->cpu.es;

			int cur_inst = seg_mem(sys->cpu.cs.whole, sys->cpu.ip.whole);

			// Only used if instruction has mod r/m byte
			// We don't know if the registers we are accesing are 8 bit or 16 bit so cast them when dealing with them
			void* reg = NULL;
			void* regmem = NULL;

			enum CPU_Opcode opcode = read_address8(sys, cur_inst, 0);
			
			if(opcode >= 0x60 && opcode <= 0x6F) // unused on 8086/8088, aliases for the jmp instructions
			{
				opcode += 0x10;
			}

			enum CPU_Opcode_Prefix opcode_prefix = read_address8(sys, cur_inst - 1, 0);

			_Bool zero_flag_check = 0;

			// This means last byte isn't part of instruction

			if(!sys->cpu.prev_byte_success)
			{
				switch (opcode_prefix) // opcode prefix
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
			}

			// In the opcodes, dd is displacement and ii is immediate and mm is mod. All of them are optional
			

			switch (opcode) // actual opcode
			{
			case GROUP_OPCODE_80:
			{
				enum CPU_Group_Opcode_80 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;

				switch (group_opcode_instruction)
				{
				case ADD_RM8_IMM8: // 80 mm ii
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

					add8(sys, regmem, imm);
					break;
				}
				case ADC_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);
					imm += sys->cpu.flag.whole & FLAG_CARRY;
					add8(sys, regmem, imm);
					break;
				}
				case SBB_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);
					imm += sys->cpu.flag.whole & FLAG_CARRY;
					sub8(sys, regmem, imm);
					break;
				}
				case AND_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

					and8(sys, regmem, imm);
					break;
				}
				case CMP_RM8_IMM8: // 80 mm ii
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

					cmp8(sys, *(uint8_t*)regmem, imm);
					break;
				}
				case SUB_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

					sub8(sys, regmem, imm);
					break;
				}
				case OR_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

					or8(sys, regmem, imm);
					break;
				}
				case XOR_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);

					xor8(sys, regmem, imm);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_81:
			{
				enum CPU_Group_Opcode_81 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case ADD_RM16_IMM16: // 81 mm ii ii
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

					add16(sys, regmem, imm);
					break;
				}
				case ADC_RM16_IMM16:
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);
					imm += sys->cpu.flag.whole & FLAG_CARRY;
					add16(sys, regmem, imm);
					break;
				}
				case SBB_RM16_IMM16:
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);
					imm += sys->cpu.flag.whole & FLAG_CARRY;
					sub16(sys, regmem, imm);
					break;
				}
				case AND_RM16_IMM16: 
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

					and16(sys, regmem, imm);
					break;
				}
				case CMP_RM16_IMM16: // 81 mm ii ii
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

					cmp16(sys, *(uint16_t*)regmem, imm);
					break;
				}
				case SUB_RM16_IMM16:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

					sub16(sys, regmem, imm);
					break;
				}
				case OR_RM16_IMM16:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

					or16(sys, regmem, imm);
					break;
				}
				case XOR_RM16_IMM16:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);

					xor16(sys, regmem, imm);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_83:
			{
				enum CPU_Group_Opcode_83 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case ADD_RM16_IMM8:
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

					add16(sys, regmem, imm);
					break;
				}
				case ADC_RM16_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);
					imm += sys->cpu.flag.whole & FLAG_CARRY;
					add16(sys, regmem, imm);
					break;
				}
				case SBB_RM16_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);
					imm += sys->cpu.flag.whole & FLAG_CARRY;
					sub16(sys, regmem, imm);
					break;
				}
				case AND_RM16_IMM8:
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

					and16(sys, regmem, imm);
					break;
				}
				case CMP_RM16_IMM8: // 83 mm ii
				{
					int16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

					cmp16(sys, *(uint16_t*)regmem, imm);
					break;
				}
				case SUB_RM16_IMM8:
				{				
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

					sub16(sys, regmem, imm);
					break;
				}
				case OR_RM16_IMM8:
				{				
					int16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

					or16(sys, regmem, imm);
					break;
				}
				case XOR_RM16_IMM8:
				{				
					int16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 0, 0);

					xor16(sys, regmem, imm);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_D0:
			{
				enum CPU_Group_Opcode_D0 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case SAL_RM8_1: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					sal8(sys, regmem, 1);
					break;
				}
				case SAR_RM8_1: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					sar8(sys, regmem, 1);
					break;
				}
				case SHR_RM8_1: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					shr8(sys, regmem, 1);
					break;
				}
				}

				break;
			}
			case GROUP_OPCODE_D1:
			{
				enum CPU_Group_Opcode_D1 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case SAL_RM16_1: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					sal16(sys, regmem, 1);
					break;
				}
				case SAR_RM16_1: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					sar16(sys, regmem, 1);
					break;
				}
				case SHR_RM16_1: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					shr16(sys, regmem, 1);
					break;
				}
				}

				break;
			}
			case GROUP_OPCODE_D2:
			{	
				enum CPU_Group_Opcode_D2 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case SAL_RM8_CL: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					sal8(sys, regmem, sys->cpu.cx.low);
					break;
				}
				case SAR_RM8_CL: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					sar8(sys, regmem, sys->cpu.cx.low);
					break;
				}
				case SHR_RM8_CL: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					shr8(sys, regmem, sys->cpu.cx.low);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_D3:
			{
				enum CPU_Group_Opcode_D3 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case SAL_RM16_CL: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					sal16(sys, regmem, sys->cpu.cx.low);
					break;
				}
				case SAR_RM16_CL: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					sar16(sys, regmem, sys->cpu.cx.low);
					break;
				}
				case SHR_RM16_CL: // D0 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					shr16(sys, regmem, sys->cpu.cx.low);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_F6:
			{
				enum CPU_Group_Opcode_F6 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case TEST_RM8_IMM8:
				{
					uint8_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 0, 0, 0);
					uint8_t temp = *(uint8_t*)regmem;
					and8(sys, &temp, imm);
					break;
				}
				case MUL_RM8: // F6 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					mul8(sys, *(uint8_t*)regmem);

					break;
				}
				case NEG_RM8:
				{	
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					neg8(sys, regmem);
					break;
				}
				case NOT_RM8:
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);

					*(uint8_t*)regmem = ~(*(uint8_t*)regmem);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_F7:
			{
				enum CPU_Group_Opcode_F7 group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case TEST_RM16_IMM16:
				{
					uint16_t imm = 0;
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm, 1, 1, 0);
					uint16_t temp = *(uint16_t*)regmem;
					and16(sys, &temp, imm);
					break;
				}
				case MUL_RM16: // F7 mm
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					mul16(sys, *(uint16_t*)regmem);

					break;
				}
				case NEG_RM16:
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					neg16(sys, regmem);
					break;
				}
				case NOT_RM16:
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					*(uint16_t*)regmem = ~(*(uint16_t*)regmem);
					break;
				}
				}
				break;
			}
			case GROUP_OPCODE_FE:
			{
				enum CPU_Group_Opcode_FE group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case DEC_RM8: // FE mm dd dd
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
					dec8(sys, regmem);
					break;
				}
				case INC_RM8: // FE mm dd dd
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
					inc8(sys, regmem);
					break;
				}
				}

				break;
			}
			case GROUP_OPCODE_FF:
			{
				enum CPU_Group_Opcode_FF group_opcode_instruction = (read_address8(sys, cur_inst + 1, 0) & 0b00111000) >> 3;
				switch (group_opcode_instruction)
				{
				case DEC_RM16: // FF mm dd dd
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
					inc16(sys, regmem);
					break;
				}
				// 0x2
				case CALL_RM16:
				{
					int call_stack_restore = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);				
					push(sys, sys->cpu.ip.whole + call_stack_restore);
					jmp(sys, sys->cpu.cs.whole, *(uint16_t*)regmem);
					break;
				}
				// 0x3
				case CALL_M16_16: // FF mm
				{
					int call_stack_restore = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

					push(sys, sys->cpu.cs.whole);
					push(sys, sys->cpu.ip.whole + call_stack_restore);

					uint16_t segment = read_address16(sys, *(uint16_t*)regmem + 2, 0);
					uint16_t offset = read_address16(sys, *(uint16_t*)regmem, 0);

					jmp(sys, segment, offset);
					break;
				}
				// 0x4
				case JMP_RM16: // FF mm dd dd
				{
					calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
					jmp(sys, sys->cpu.cs.whole, *(uint16_t*)regmem);
					break;
				}
				// 0x6
				case PUSH_RM16: // FF mm dd dd
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
					push(sys, *(uint16_t*)regmem);
					break;
				}
				// 0x0
				case INC_RM16: // FF mm dd dd
				{
					ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
					inc16(sys, regmem);
					break;
				}
				}

				break;
			}
			case ADC_AL_IMM8:
			{
				uint8_t imm = read_address8(sys, cur_inst + 1, 0);
				sys->cpu.ax.low += imm + sys->cpu.flag.whole & FLAG_CARRY;
				ip_increase += 2;
				break;
			}
			case ADC_AX_IMM16:
			{
				uint16_t imm = read_address16(sys, cur_inst + 1, 0);
				sys->cpu.ax.whole += imm + sys->cpu.flag.whole & FLAG_CARRY;
				ip_increase += 2;
				break;
			}
			case ADC_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				*(uint8_t*)reg += sys->cpu.flag.whole & FLAG_CARRY;
				add8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case ADC_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				*(uint16_t*)reg += sys->cpu.flag.whole & FLAG_CARRY;
				add16(sys, regmem, *(uint16_t*)reg);
				break;
			}
			case ADC_R8_RM8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				*(uint8_t*)regmem += sys->cpu.flag.whole & FLAG_CARRY;
				add8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case ADC_R16_RM16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				*(uint16_t*)regmem += sys->cpu.flag.whole & FLAG_CARRY;
				add16(sys, reg, *(uint16_t*)regmem);
				break;
			}
			case ADD_RM8_R8: // 00 mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				add8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case ADD_RM16_R16: // 01 mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				add16(sys, regmem, *(uint16_t*)reg);
				break;
			}
			case ADD_R8_RM8: // 02 mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				add8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case ADD_R16_RM16: // 03 mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				add16(sys, reg, *(uint16_t*)regmem);
				break;
			}
			case ADD_AL_IMM8: // 04 ii
			{
				add8(sys, &sys->cpu.ax.low, read_address8(sys, cur_inst + 1, 0));
				ip_increase = 2;
				break;
			}
			case ADD_AX_IMM16: // 05 ii ii
			{
				add16(sys, &sys->cpu.ax.whole, read_address16(sys, cur_inst + 1, 0));
				ip_increase = 3;
				break;
			}
			case AND_AL_IMM8:
			{
				uint8_t imm = read_address8(sys, cur_inst + 1, 0);
				and8(sys, &sys->cpu.ax.low, imm);
				ip_increase = 2;
				break;
			}
			case AND_AX_IMM16:
			{
				uint16_t imm = read_address16(sys, cur_inst + 1, 0);
				and16(sys, &sys->cpu.ax.whole, imm);
				ip_increase = 3;
				break;
			}
			case AND_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				and8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case AND_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				and16(sys, regmem, *(uint16_t*)reg);
				break;
			}
			case AND_R8_RM8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				and8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case AND_R16_RM16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				and16(sys, reg, *(uint16_t*)regmem);
				break;
			}
			case CALL_PTR16_16: // 9A ii ii ii ii
			{
				push(sys, sys->cpu.cs.whole);
				push(sys, sys->cpu.ip.whole);

				uint16_t offset = read_address16(sys, cur_inst + 1, 0);
				uint16_t segment = read_address16(sys, cur_inst + 3, 0);

				jmp(sys, segment, offset);

				break;
			}
			case CALL_REL16: // E8 ii ii
			{
				push(sys, sys->cpu.ip.whole + 3);

				int16_t call_value = read_address16(sys, cur_inst + 1, 0) + 3;

				jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + call_value);

				break;
			}
			case CBW:
			{
				int16_t new_ax = sys->cpu.ax.low;
				sys->cpu.ax.whole = new_ax;
				ip_increase = 1;
				break;
			}
			case CLC:
			{
				sys->cpu.flag.whole &= ~FLAG_CARRY;
				ip_increase = 1;
				break;
			}
			case CLD:
			{
				sys->cpu.flag.whole &= ~FLAG_DIRECTION;
				ip_increase = 1;
				break;
			}
			case CLI:
			{
				sys->cpu.flag.whole &= ~FLAG_INTERRUPT;
				ip_increase = 1;
				break;
			}
			case CMC:
			{
				if(sys->cpu.flag.whole & FLAG_CARRY)
				{
					sys->cpu.flag.whole &= ~FLAG_CARRY;
				}

				else
				{
					sys->cpu.flag.whole |= FLAG_CARRY;
				}

				ip_increase = 1;
				break;
			}
			case CMP_AL_IMM8: // 3C ii
			{
				cmp8(sys, sys->cpu.ax.low, read_address8(sys, cur_inst + 1, 0));
				ip_increase = 2;
				break;
			}
			case CMP_AX_IMM16: // 3D ii ii
			{
				cmp16(sys, sys->cpu.ax.whole, read_address16(sys, cur_inst + 1, 0));
				ip_increase = 3;
				break;
			}
			case CMP_RM8_R8: // 38 mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				cmp8(sys, *(uint8_t*)regmem, *(uint8_t*)reg);
				break;
			}
			case CMP_RM16_R16: // 39 mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				cmp16(sys, *(uint16_t*)regmem, *(uint16_t*)reg);
				break;
			}
			case CMP_R8_RM8: // 3A mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				cmp8(sys, *(uint8_t*)reg, *(uint8_t*)regmem);
				break;
			}
			case CMP_R16_RM16: // 3B mm
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				cmp8(sys, *(uint16_t*)reg, *(uint16_t*)regmem);
				break;
			}
			case CMPSB:
			{
				cmp8(sys, read_address8(sys, seg_mem(sys->cpu.ds.whole, sys->cpu.si.whole), 0), read_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), 0));

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.si.whole--;
					sys->cpu.di.whole--;
				}
				
				else
				{
					sys->cpu.si.whole++;
					sys->cpu.di.whole++;
				}

				ip_increase = 1;
				zero_flag_check = 1;
				break;
			}
			case CMPSW:
			{
				cmp16(sys, read_address16(sys, seg_mem(sys->cpu.ds.whole, sys->cpu.si.whole), 0), read_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), 0));

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.si.whole -= 2;
					sys->cpu.di.whole -= 2;
				}
				
				else
				{
					sys->cpu.si.whole += 2;
					sys->cpu.di.whole += 2;
				}

				ip_increase = 1;
				zero_flag_check = 1;
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
				int reg_code = opcode - DEC_AX;

				reg = reg16_index(&sys->cpu, reg_code);

				dec16(sys, reg);
				ip_increase = 1;
				break;
			}
			case HLT:
			{
				sys->cpu.halted = 1;
				ip_increase = 1;
				break;
			}
			case IN_AL_IMM8:
			{
				uint8_t address = read_address8(sys, cur_inst + 1, 0);
				sys->cpu.ax.low = read_address8(sys, address, 1);
				ip_increase = 2;
				break;
			}
			case IN_AX_IMM8:
			{
				uint8_t address = read_address8(sys, cur_inst + 1, 0);
				sys->cpu.ax.whole = read_address16(sys, address, 1);
				ip_increase = 2;
				break;
			}
			case IN_AL_DX:
			{
				sys->cpu.ax.low = read_address8(sys, sys->cpu.dx.whole, 1);
				ip_increase = 1;
				break;
			}
			case IN_AX_DX:
			{
				sys->cpu.ax.whole = read_address16(sys, sys->cpu.dx.whole, 1);
				ip_increase = 1;
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
				int reg_code = opcode - INC_AX;

				reg = reg16_index(&sys->cpu, reg_code);

				inc16(sys, reg);

				ip_increase = 1;
				break;
			}
			case INT_IMM8: // CD ii
			{
				uint8_t interrupt = read_address8(sys, cur_inst + 1, 0);

				// Look up in interrupt vector table. Example: int 0x10, 0x10 * 4 = 0x40, get offset at 0x40 and segment at 0x42, jump there 

				uint16_t interrupt_offset = read_address16(sys, seg_mem(0, interrupt * 4), 0);
				uint16_t interrupt_segment = read_address16(sys, seg_mem(0, interrupt * 4) + 2, 0);

				push(sys, sys->cpu.flag.whole);
				push(sys, sys->cpu.cs.whole);
				push(sys, sys->cpu.ip.whole + 2);

				jmp(sys, interrupt_segment, interrupt_offset);

				break;
			}
			case IRET: // CF
			{
				// pop ip, cs, and flags after interrupt

				pop(sys, &sys->cpu.ip.whole);
				pop(sys, &sys->cpu.cs.whole);
				pop(sys, &sys->cpu.flag.whole);

				break;
			}
			case JA_REL8: // 77 ii
			{
				if ((sys->cpu.flag.whole & FLAG_CARRY) == 0 && (sys->cpu.flag.whole & FLAG_ZERO) == 0)
				{
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
				}

				else
				{
					ip_increase += 2;
				}

				break;
			}
			case JP_REL8: // 7A ii // Same as JPE_REL8
			{
				if (sys->cpu.flag.whole & FLAG_PARITY)
				{
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
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
					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + (int8_t)read_address8(sys, cur_inst + 1, 0) + 2);
				}

				else
				{
					ip_increase += 2;
				}

				break;
			}
			case JMP_REL8: // EB ii
			{
				int8_t jmp_value = read_address8(sys, cur_inst + 1, 0) + 2;
				jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + jmp_value);
				break;
			}
			case JMP_REL16: // E9 ii ii
			{
				int16_t jmp_value = read_address16(sys, cur_inst + 1, 0) + 3;
				jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + jmp_value);
				break;
			}
			case JMP_PTR16_16: // EA ii ii ii ii
			{
				uint16_t new_ip = read_address16(sys, cur_inst + 1, 0);
				uint16_t new_cs = read_address16(sys, cur_inst + 3, 0);

				jmp(sys, new_cs, new_ip);
				break;
			}
			case LES:
			{
				uint16_t imm = 0;
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);

				*(uint16_t*)reg = *(uint16_t*)regmem;
				((uint16_t*)regmem)++;
				sys->cpu.es.whole = *(uint16_t*)regmem;

				break;
			}
			case LODSB:
			{
				sys->cpu.ax.low = read_address8(sys, seg_mem(sys->cpu.ds.whole, sys->cpu.si.whole), 0);

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.si.whole--;
				}
				
				else
				{
					sys->cpu.si.whole++;
				}

				ip_increase = 1;
				break;
			}
			case LODSW:
			{
				sys->cpu.ax.whole = read_address16(sys, seg_mem(sys->cpu.ds.whole, sys->cpu.si.whole), 0);

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.si.whole -= 2;
				}
				
				else
				{
					sys->cpu.si.whole += 2;
				}

				ip_increase = 1;
				break;
			}
			case LOOP_REL8:
			{
				sys->cpu.cx.whole--;

				if(sys->cpu.cx.whole != 0)
				{
					int8_t jmp_value = read_address8(sys, cur_inst + 1, 0) + 2;

					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + jmp_value);
				}

				else
				{
					ip_increase += 2;
				}

				break;
			}
			case LOOPE_REL8:
			{
				sys->cpu.cx.whole--;
				
				if(sys->cpu.cx.whole != 0 && sys->cpu.flag.whole & FLAG_ZERO)
				{
					int8_t jmp_value = read_address8(sys, cur_inst + 1, 0) + 2;

					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + jmp_value);
				}

				else
				{
					ip_increase += 2;
				}

				break;
			}
			case LOOPNE_REL8:
			{
				sys->cpu.cx.whole--;
				
				if(sys->cpu.cx.whole != 0 && (sys->cpu.flag.whole & FLAG_ZERO) == 0)
				{
					int8_t jmp_value = read_address8(sys, cur_inst + 1, 0) + 2;

					jmp(sys, sys->cpu.cs.whole, sys->cpu.ip.whole + jmp_value);
				}

				else
				{
					ip_increase += 2;
				}
				
				break;
			}
			case MOV_RM8_R8: // 88 mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				mov8(sys, regmem, reg);
				break;
			}
			case MOV_RM16_R16: // 89 mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				mov16(sys, regmem, reg);
				break;
			}
			case MOV_R8_RM8: // 8A mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				mov8(sys, reg, regmem);
				break;
			}
			case MOV_R16_RM16: // 8B mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				mov16(sys, reg, regmem);
				break;
			}
			case MOV_RM16_SREG: // 8C mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 1);
				mov16(sys, regmem, reg);
				break;
			}
			case MOV_SREG_RM16: // 8E mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 1);
				mov16(sys, reg, regmem);
				break;
			}
			case MOV_AL_MOFFS8: // A0 dd dd
			{
				uint16_t moffs8 = read_address16(sys, cur_inst + 1, 0);
				uint8_t moffs8_value = read_address8(sys, seg_mem(data_seg->whole, moffs8), 0);
				mov8(sys, &sys->cpu.ax.low, &moffs8_value);
				ip_increase = 3;
				break;
			}
			case MOV_AX_MOFFS16: // A1 dd dd
			{
				uint16_t moffs16 = read_address16(sys, cur_inst + 1, 0);
				uint16_t moffs16_value = read_address16(sys, seg_mem(data_seg->whole, moffs16), 0);
				mov16(sys, &sys->cpu.ax.whole, &moffs16_value);
				ip_increase = 3;
				break;
			}
			case MOV_MOFFS8_AL: // A2 dd dd
			{
				uint16_t moffs8 = read_address16(sys, cur_inst + 1, 0);
				write_address8(sys, seg_mem(data_seg->whole, moffs8), sys->cpu.ax.low, 0);
				ip_increase = 3;
				break;
			}
			case MOV_MOFFS16_AX: // A2 dd dd
			{
				uint16_t moffs16 = read_address16(sys, cur_inst + 1, 0);
				write_address16(sys, seg_mem(data_seg->whole, moffs16), sys->cpu.ax.whole, 0);
				ip_increase = 3;
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
			{
				int reg_code = opcode - MOV_AL_IMM8;

				reg = reg8_index(&sys->cpu, reg_code);

				uint8_t imm8 = read_address8(sys, cur_inst + 1, 0);
				mov8(sys, reg, &imm8);

				ip_increase = 2;
				break;
			}
			case MOV_AX_IMM16: // B8+x ii ii
			case MOV_CX_IMM16:
			case MOV_DX_IMM16:
			case MOV_BX_IMM16:
			case MOV_SP_IMM16:
			case MOV_BP_IMM16:
			case MOV_SI_IMM16:
			case MOV_DI_IMM16:
			{
				int reg_code = opcode - MOV_AX_IMM16;

				reg = reg16_index(&sys->cpu, reg_code);

				uint16_t imm16 = read_address16(sys, cur_inst + 1, 0);

				mov16(sys, reg, &imm16);

				ip_increase = 3;
				break;
			}
			case MOV_RM8_IMM8: // C6 mm dd dd ii
			{
				uint8_t imm8 = 0;

				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm8, 0, 0, 0);

				mov8(sys, regmem, &imm8);
				break;
			}
			case MOV_RM16_IMM16: // C7 mm dd dd ii ii
			{
				uint16_t imm16 = 0;

				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, &imm16, 1, 1, 0);

				mov16(sys, regmem, &imm16);

				break;
			}
			case MOVSB:
			{
				write_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), read_address8(sys, seg_mem(sys->cpu.ds.whole, sys->cpu.si.whole), 0), 0);

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.si.whole--;
					sys->cpu.di.whole--;
				}
				
				else
				{
					sys->cpu.si.whole++;
					sys->cpu.di.whole++;
				}

				ip_increase = 1;
				break;
			}
			case MOVSW:
			{
				write_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), read_address16(sys, seg_mem(sys->cpu.ds.whole, sys->cpu.si.whole), 0), 0);

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.si.whole -= 2;
					sys->cpu.di.whole -= 2;
				}
				
				else
				{
					sys->cpu.si.whole += 2;
					sys->cpu.di.whole += 2;
				}

				ip_increase = 1;
				break;
			}
			case OR_AL_IMM8:
			{
				or8(sys, &sys->cpu.ax.low, read_address8(sys, cur_inst + 1, 0));
				ip_increase = 2;
				break;
			}
			case OR_AX_IMM16:
			{
				or16(sys, &sys->cpu.ax.whole, read_address16(sys, cur_inst + 1, 0));
				ip_increase = 3;
				break;
			}
			case OR_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				or8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case OR_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				or16(sys, regmem, *(uint16_t*)reg);
				break;
			}
			case OR_R8_RM8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				or8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case OR_R16_RM16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				or16(sys, reg, *(uint16_t*)regmem);
				break;
			}
			case OUT_IMM8_AL:
			{
				uint8_t address = read_address8(sys, cur_inst + 1, 0);
				write_address8(sys, address, sys->cpu.ax.low, 1);
				ip_increase = 2;
				break;
			}
			case OUT_IMM8_AX:
			{
				uint8_t address = read_address8(sys, cur_inst + 1, 0);
				write_address16(sys, address, sys->cpu.ax.whole, 1);
				ip_increase = 2;
				break;
			}
			case OUT_DX_AL:
			{
				write_address8(sys, sys->cpu.dx.whole, sys->cpu.ax.low, 1);
				ip_increase = 1;
				break;
			}
			case OUT_DX_AX:
			{
				write_address8(sys, sys->cpu.dx.whole, sys->cpu.ax.whole, 1);
				ip_increase = 1;
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
			{
				int reg_code = opcode - PUSH_AX;

				reg = reg16_index(&sys->cpu, reg_code);

				push(sys, *(uint16_t*)reg);

				ip_increase = 1;
				break;
			}
			// 06
			case PUSH_ES: // es
			case PUSH_CS: // cs
			case PUSH_SS: // ss
			case PUSH_DS: // ds
			{
				switch (opcode)
				{
				case PUSH_ES:
			{
				sys->cpu.sp.whole -= 2;

				if ((opcode - 0x8) == POP_SREG)
				{
					reg = &sys->cpu.cs.whole;
					break;
				}
				case PUSH_SS:
				{
					reg = &sys->cpu.ss.whole;
					break;
				}
				case PUSH_DS:
				{
					reg = &sys->cpu.ds.whole;
					break;
				}

				else
				{
					reg = &sys->cpu.es.whole;
				}

				push(sys, reg);

				ip_increase = 1;
				break;
			}
			case PUSHF:
			{
				push(sys, sys->cpu.flag.whole);
				ip_increase = 1;
				break;
			}
			case POP_RM16: // 8F mm dd dd
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				push(sys, *(uint16_t*)regmem);
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
			{
				int reg_code = opcode - POP_AX;

				reg = reg16_index(&sys->cpu, reg_code);

				pop(sys, reg);

				ip_increase = 1;
				break;
			}
			// 07
			case POP_ES: // es
			case POP_CS: // cs
			case POP_SS: // ss
			case POP_DS: // ds
			{
				switch (opcode)
				{
				case POP_ES:
			{
					reg = &sys->cpu.es.whole;
					break;
				}
				case POP_CS:
				{
					reg = &sys->cpu.cs.whole;
					break;
				}
				case POP_SS:
				{
					reg = &sys->cpu.ss.whole;
					break;
				}
				case POP_DS:
				{
					reg = &sys->cpu.ds.whole;
					break;
				}
				}

				pop(sys, reg);

				ip_increase = 1;
				break;
			}
			case POPF:
			{
				pop(sys, &sys->cpu.flag.whole);
				ip_increase = 1;
				break;
			}
			case RET_FAR:
			{
				pop(sys, &sys->cpu.ip.whole);
				pop(sys, &sys->cpu.cs.whole);
				break;
			}			
			case RET_FAR_IMM16:
			{
				pop(sys, &sys->cpu.ip.whole);
				pop(sys, &sys->cpu.cs.whole);
				sys->cpu.sp.whole += read_address16(sys, cur_inst + 1, 0);
				break;
			}
			case RET_NEAR:
			{
				pop(sys, &sys->cpu.ip.whole);
				break;
			}
			case RET_NEAR_IMM16:
			{
				pop(sys, &sys->cpu.ip.whole);
				sys->cpu.sp.whole += read_address16(sys, cur_inst + 1, 0);
				break;
			}
			case SBB_AL_IMM8:
			{
				uint8_t imm = read_address8(sys, cur_inst + 1, 0);
				imm += sys->cpu.flag.whole & FLAG_CARRY;
				sub8(sys, &sys->cpu.ax.low, imm);
				break;
			}
			case SBB_AX_IMM16:
			{
				uint16_t imm = read_address16(sys, cur_inst + 1, 0);
				imm += sys->cpu.flag.whole & FLAG_CARRY;
				sub8(sys, &sys->cpu.ax.whole, imm);
				break;
			}
			case SBB_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				(*(uint8_t*)reg) += sys->cpu.flag.whole & FLAG_CARRY;
				sub8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case SBB_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				(*(uint16_t*)reg) += sys->cpu.flag.whole & FLAG_CARRY;
				sub8(sys, regmem, (*(uint16_t*)reg));
				break;
			}
			case SBB_R8_RM8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				(*(uint8_t*)regmem) += sys->cpu.flag.whole & FLAG_CARRY;
				sub8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case SBB_R16_RM16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				(*(uint16_t*)regmem) += sys->cpu.flag.whole & FLAG_CARRY;
				sub8(sys, reg, (*(uint16_t*)regmem));
				break;
			}
			case STC:
			{
				sys->cpu.flag.whole |= FLAG_CARRY;
				ip_increase = 1;
				break;
			}
			case STD:
			{
				sys->cpu.flag.whole |= FLAG_DIRECTION;
				ip_increase = 1;
				break;
			}
			case STI:
			{
				sys->cpu.flag.whole |= FLAG_INTERRUPT;
				ip_increase = 1;
				break;
			}
			case STOSB:
			{
				write_address8(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), sys->cpu.ax.low, 0);

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.di.whole--;
				}
				
				else
				{
					sys->cpu.di.whole++;
				}

				ip_increase = 1;
				break;
			}
			case STOSW:
			{
				write_address16(sys, seg_mem(sys->cpu.es.whole, sys->cpu.di.whole), sys->cpu.ax.whole, 0);

				if(sys->cpu.flag.whole & FLAG_DIRECTION)
				{
					sys->cpu.di.whole -= 2;
				}
				
				else
				{
					sys->cpu.di.whole += 2;
				}

				ip_increase = 1;
				break;
			}
			case SUB_AL_IMM8: // 2C ii
			{
				sub8(sys, &sys->cpu.ax.low, read_address8(sys, cur_inst + 1, 0));
				ip_increase = 2;
				break;
			}
			case SUB_AX_IMM16: // 2D ii ii
			{
				sub16(sys, &sys->cpu.ax.whole, read_address16(sys, cur_inst + 1, 0));
				ip_increase = 3;
				break;
			}
			case SUB_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				sub8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case SUB_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				sub16(sys, regmem, *(uint16_t*)reg);
				break;
			}
			case SUB_R8_RM8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				sub8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case SUB_R16_RM16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				sub16(sys, reg, *(uint16_t*)regmem);
				break;
			}
			case TEST_AL_IMM8:
			{
				uint8_t temp = sys->cpu.ax.low;
				uint8_t imm = read_address8(sys, cur_inst + 1, 0);
				and8(sys, &temp, imm);
				ip_increase = 2;
				break;
			}
			case TEST_AX_IMM16:
			{
				uint16_t temp = sys->cpu.ax.low;
				uint16_t imm = read_address16(sys, cur_inst + 1, 0);
				and16(sys, &temp, imm);
				ip_increase = 3;
				break;
			}
			case TEST_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				uint8_t temp = *(uint8_t*)regmem;
				and8(sys, &temp, *(uint8_t*)reg);
				break;
			}
			case TEST_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				uint16_t temp = *(uint16_t*)regmem;
				and16(sys, &temp, *(uint16_t*)reg);
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
				int reg_code = opcode - XCHG_AX_AX;

				reg = reg16_index(&sys->cpu, reg_code);

				uint16_t temp = *(uint16_t*)reg;

				mov16(sys, reg, &sys->cpu.ax.whole);
				mov16(sys, &sys->cpu.ax.whole, &temp);

				ip_increase = 1;
				break;
			}
			case XCHG_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				
				uint8_t temp = *(uint8_t*)reg;

				mov8(sys, reg, regmem);
				mov8(sys, regmem, &temp);
				break;
			}
			case XCHG_RM16_R16:
			{	
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				
				uint16_t temp = *(uint16_t*)reg;

				mov16(sys, reg, regmem);
				mov16(sys, regmem, &temp);
				break;
			}
			case XOR_AL_IMM8:
			{
				uint8_t imm = read_address8(sys, cur_inst + 1, 0);
				xor8(sys, &sys->cpu.ax.low, imm);
				ip_increase = 2;
				break;
			}
			case XOR_AX_IMM16:
			{
				uint8_t imm = read_address16(sys, cur_inst + 1, 0);
				xor16(sys, &sys->cpu.ax.whole, imm);
				ip_increase = 3;
				break;
			}
			case XOR_RM8_R8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				
				xor8(sys, regmem, *(uint8_t*)reg);
				break;
			}
			case XOR_RM16_R16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				
				xor16(sys, regmem, *(uint16_t*)reg);
				break;
			}
			case XOR_R8_RM8:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 0, 0, 0);
				
				xor8(sys, reg, *(uint8_t*)regmem);
				break;
			}
			case XOR_R16_RM16:
			{
				ip_increase = calc_modrm_byte(sys, data_seg, cur_inst, &reg, &regmem, NULL, 1, 0, 0);
				
				xor16(sys, reg, *(uint16_t*)regmem);
				break;
			}
			default:
				if(opcode != PREFIX_ES && opcode != PREFIX_DS && opcode != PREFIX_CS && opcode != PREFIX_SS && opcode != PREFIX_REPNE && opcode != PREFIX_REP_OR_REPE)
				{
					printf("Unknown Opcode %x\n", opcode);
				}

				sys->cpu.prev_byte_success = 0;
				sys->cpu.ip.whole++;
				return;
			}

			_Bool done = 0;

			if(opcode_prefix == PREFIX_REPNE)
			{
				if((sys->cpu.flag.whole & FLAG_ZERO) == 0 && sys->cpu.cx.whole != 0)
				{
					ip_increase = 0;
				}

				else
				{
					done = 1;
				}
			}

			else if(opcode_prefix == PREFIX_REP_OR_REPE)
			{
				if(zero_flag_check)
				{
					if(sys->cpu.flag.whole & FLAG_ZERO && sys->cpu.cx.whole != 0)
					{
						ip_increase = 0;
					}
					
					else
					{
						done = 1;
					}
				}
			}

			if(opcode_prefix == PREFIX_REP_OR_REPE || opcode_prefix == PREFIX_REPNE)
			{
				if(sys->cpu.cx.whole != 0 && !done)
				{
					ip_increase = 0;
					sys->cpu.cx.whole--;
				}
			}

			sys->cpu.prev_byte_success = 1;
			sys->cpu.ip.whole += ip_increase;
		}
	}
}
