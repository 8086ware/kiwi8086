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
	Pic pic_master;
	Pic pic_slave;
	Pit pit;
	Ppi ppi;
	DMA dma;
} Sys8086;

Sys8086* init_sys(FILE* image, FILE* bios_rom);

int cpu_process_opcode(Sys8086* sys, enum CPU_Opcode opcode, Register* data_seg, int cur_inst);
void cpu_exec(Sys8086* sys);
uint8_t calc_modrm_byte(Sys8086* sys, Register* data_seg, int instruction_address, void** reg, void** regmem, void* imm, _Bool word, _Bool imm_word, _Bool sreg);
void pic_check_int(Sys8086* sys);
void pit_cycle(Sys8086* sys);
void display_render(Sys8086* sys);

uint8_t handle_pic_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_cga_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_crtc_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_pit_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_ppi_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);
uint8_t handle_dma_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read);

#endif
