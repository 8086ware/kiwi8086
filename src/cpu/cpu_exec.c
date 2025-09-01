#include "system.h"
#include "cpu/instructions.h"
#include "memory.h"
#include "pic.h"

void cpu_exec(Sys8086* sys)
{
	uint64_t now_ticks = SDL_GetTicksNS();

	int instructions_to_do = (now_ticks - sys->cpu.last_tick) / CPU_NANOSECONDS_PER_CYCLE;

	sys->cpu.last_tick = now_ticks;

	if (instructions_to_do > 5'000'000)
	{
		instructions_to_do = 5'000'000;
	}

	for (int i = 0; i < instructions_to_do; i++)
	{
		sys->cpu.instructions++; // measuring ips

		pic_check_int(sys);

		// Fetch, Decode and execute instruction
		if (!sys->cpu.halted)
		{
			Register temp_reg = sys->cpu.ds; // In modrm, 
			Register* data_seg = &temp_reg; // Default data segment

			int cur_inst = seg_mem(sys->cpu.cs.whole, sys->cpu.ip.whole);

			_Bool instruction_done = 0;

			_Bool zero_flag_check = 0;

			_Bool repeat = 0;

			int instruction_i = 0;

			while(!instruction_done)
			{
				enum CPU_Opcode cur_byte = read_address8(sys, cur_inst + instruction_i, 0);

				switch (cur_byte) // opcode prefix?
				{
				case 0xf0:
				{
					instruction_i++;
					break;
				}
				case PREFIX_REP_OR_REPE:
				{
					if (sys->cpu.zero_flag_check)
					{
						if ((sys->cpu.flag.whole & FLAG_ZERO) && sys->cpu.cx.whole != 0)
						{
							sys->cpu.cx.whole--;
							repeat = 1;
						}
						sys->cpu.zero_flag_check = 0;
					}

					else
					{
						if (sys->cpu.cx.whole != 0)
						{
							sys->cpu.cx.whole--;
							repeat = 1;
						}

					}

					instruction_i++;

					break;
				}
				case PREFIX_REPNE:
				{
					if ((sys->cpu.flag.whole & FLAG_ZERO) == 0 && sys->cpu.cx.whole != 0)
					{
						sys->cpu.cx.whole--;
						repeat = 1;
					}

					instruction_i++;

					break;
				}
				case PREFIX_ES:
				{
					data_seg = &sys->cpu.es;
					instruction_i++;
					break;
				}
				case PREFIX_CS:
				{
					data_seg = &sys->cpu.cs;
					instruction_i++;
					break;
				}
				case PREFIX_SS:
				{
					data_seg = &sys->cpu.ss;
					instruction_i++;
					break;
				}
				case PREFIX_DS:
				{
					data_seg = &sys->cpu.ds;
					instruction_i++;
					break;
				}
				default:
				{
					int new_ip_increase = 0;
					new_ip_increase = cpu_process_opcode(sys, cur_byte, data_seg, cur_inst + instruction_i);

					if (new_ip_increase == 0) // A return value of 0 means we ain't increasing shit no matter what (call, ret, jmp, etc)
					{
						instruction_i = 0;
					}

					else
					{
						instruction_i += new_ip_increase;
					}

					instruction_done = 1;
					break;
				}
				}
			}

			if (!repeat)
			{
				sys->cpu.ip.whole += instruction_i;
			}
		}
	}
}
