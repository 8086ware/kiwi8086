#include "system.h"
#include "cpu/instructions.h"
#include "memory.h"
#include "pic.h"

void cpu_exec(Sys8086* sys)
{
	sys->cpu.instructions++; // measuring ips

	if (sys->cpu.sti_enable)
	{
		sys->cpu.flag.whole |= FLAG_INTERRUPT;
		sys->cpu.sti_enable = 0;
	}

	// Fetch, Decode and execute instruction

	Instruction instruction;
	memset(&instruction, 0, sizeof(Instruction));

	if (!sys->cpu.halted)
	{
		cpu_get_instruction(sys, &instruction);

		if (instruction.rep == PREFIX_REP_OR_REPE)
		{
			if (sys->cpu.zero_flag_check)
			{
				if ((sys->cpu.flag.whole & FLAG_ZERO) && sys->cpu.cx.whole != 0)
				{
					cpu_exec_instruction(sys, &instruction);
					sys->cpu.cx.whole--;
				}

				else
				{
					sys->cpu.ip.whole += instruction.length;
				}

				sys->cpu.zero_flag_check = 0;
			}

			else
			{
				if (sys->cpu.cx.whole != 0)
				{
					cpu_exec_instruction(sys, &instruction);
					sys->cpu.cx.whole--;
				}

				else
				{
					sys->cpu.ip.whole += instruction.length;
				}
			}
		}

		else if(instruction.rep == PREFIX_REPNE)
		{
			if ((sys->cpu.flag.whole & FLAG_ZERO) == 0 && sys->cpu.cx.whole != 0)
			{
				cpu_exec_instruction(sys, &instruction);
				sys->cpu.cx.whole--;
			}

			else
			{
				sys->cpu.ip.whole += instruction.length;
			}
		}

		else
		{
			if (!cpu_exec_instruction(sys, &instruction))
			{
				sys->cpu.ip.whole += instruction.length;
			}
		}
	}

	pic_check_int(sys);
}
