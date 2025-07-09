#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "cpu.h"
#include <stdio.h>
#include "vga.h"

#define MAX_MEMORY_8086 1'048'576

typedef struct Sys8086
{
	VGA display;
	uint8_t* memory;
	CPU cpu;
} Sys8086;

Sys8086* init_sys(FILE* image);
void cpu_exec(Sys8086* sys);
uint8_t calc_modrm_byte(Sys8086* sys, Register* data_seg, int instruction_address, void** reg, void** regmem, void* imm, _Bool word, _Bool imm_word, _Bool sreg);
uint32_t seg_mem(uint16_t seg, uint16_t offset);

#endif