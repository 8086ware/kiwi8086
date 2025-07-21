#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "cpu/cpu.h"
#include <stdio.h>
#include "display/mda.h"
#include "8259pic.h"

#define MAX_MEMORY_8086 1048576
// Things to emulate:
// 8086 CPU
// 1 Megabyte memory
// MDA Display
// I8042 PS/2 Controller
// 8259 Programmable Interrupt Controller
// 8271 Floppy disk controller
// Intel 8253/8254 Programmable Interval Timer
// 8237 DMA Controller

typedef struct Sys8086
{
	MDA display;
	uint8_t* memory;
	CPU cpu;
	pic8259 pic_master;
	pic8259 pic_slave;
} Sys8086;

Sys8086* init_sys(FILE* image, FILE* bios_rom);
void cpu_exec(Sys8086* sys);
uint8_t calc_modrm_byte(Sys8086* sys, Register* data_seg, int instruction_address, void** reg, void** regmem, void* imm, _Bool word, _Bool imm_word, _Bool sreg);

#endif
