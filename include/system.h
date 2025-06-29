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

#endif