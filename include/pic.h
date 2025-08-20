#ifndef PIC_H
#define PIC_H

#include <stdint.h>

enum Pic_Port
{
	PIC_MASTER_COMMAND_PORT = 0x20,
	PIC_MASTER_DATA_PORT = 0x21,
	PIC_SLAVE_COMMAND_PORT = 0xA0,
	PIC_SLAVE_DATA_PORT = 0xA1,
};

enum Pic_IRQ
{
	PIC_IRQ_PIT = 0,
	PIC_IRQ_KEYBOARD = 1,
	PIC_IRQ_SLAVE_CONTROLLER = 2,
	PIC_IRQ_COM2_OR_COM4 = 3,
	PIC_IRQ_COM1_OR_COM3 = 4,
	PIC_IRQ_LPT2 = 5,
	PIC_IRQ_FLOPPY_CTRL = 6,
	PIC_IRQ_LPT1 = 7,
	PIC_IRQ_RTC = 8,
	PIC_IRQ_MOUSE = 12,
	PIC_IRQ_MATH_COPROCESSOR = 13,
	PIC_IRQ_HARD_DISK_CTRL1 = 14,
	PIC_IRQ_HARD_DISK_CTRL2 = 15,
};

enum Pic_ICW // Tell this to each PIC.
{
	PIC_ICW1_ICW4 = 0x01,
	PIC_ICW1_SINGLE = 0x02,
	PIC_ICW1_INIT = 0x10, // imr cleared, irq 7 assigned priority 7, slave mode address is set to 7, status read set to irr

	// ICW2 is the vector offsets
	// ICW3 for setting the slave pic irq

	ICW4_8086 = 0x01,
	ICW4_AUTO = 0x02,
};

typedef struct Pic
{	
	uint8_t command;
	// -1 if no interrupt, > -1 contains interrupt number
	uint8_t isr; // in service register, x bit is currently 
	uint8_t irr; // interrupt request register, x bit is raised irq
	uint8_t imr; // interrupt mask register, will prevent any irq at x bit set
	uint16_t vector_offset; // offset from 0000:0000, or the ivt

	uint8_t icw_step;
} Pic;


#endif
