#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "cpu/cpu.h"
#include <stdio.h>
#include "display/display.h"
#include "pic.h"
#include "pit.h"
#include "ppi.h"
#include "dma.h"
#include "fdc.h"

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
	Display display;
	uint8_t memory[MAX_MEMORY_8086];
	CPU cpu;
	Pic pic;
	Pit pit;
	Ppi ppi;
	DMA dma;
	FDC fdc;
} Sys8086;

Sys8086* init_sys(FILE* image, FILE* bios_rom);

int cpu_exec_instruction(Sys8086* sys, Instruction* instruction);
void cpu_get_instruction(Sys8086* sys, Instruction* instruction);
void cpu_cycle(Sys8086* sys);
void calc_modrm_byte(Sys8086* sys, Instruction* instruction, int modrm_address, _Bool sreg);
void pic_check_int(Sys8086* sys);
void pit_cycle(Sys8086* sys);
void display_render(Sys8086* sys);

uint8_t handle_pic_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_cga_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_crtc_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_pit_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_ppi_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_dma_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_fdc_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);

#endif
