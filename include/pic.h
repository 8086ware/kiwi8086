#ifndef PIC_H
#define PIC_H

#include <stdint.h>

enum Pic_Port
{
	PIC_MASTER_COMMAND_PORT = 0x20,
	PIC_MASTER_DATA_PORT = 0x21,
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
};

enum Pic_ICW // Tell this to each PIC.
{
	PIC_ICW1_ICW4 = 0x01,
	PIC_ICW1_SINGLE = 0x02,
	PIC_ICW1_INIT = 0x10, // imr cleared, irq 7 assigned priority 7, slave mode address is set to 7, status read set to irr

	// ICW2 is the vector offsets
	// ICW3 for setting the slave pic irq

	PIC_ICW4_8086 = 0x01,
	PIC_ICW4_AUTO = 0x02,
};

enum Pic_OCW // Tell this to each PIC... AFTER INITALIZING
{
	PIC_OCW2_SERVICING = 0x7,
	PIC_OCW2_EOI = 0x20,
	PIC_OCW2_SL = 0x40,
	PIC_OCW2_R = 0x80,

	PIC_OCW3_READ_REGISTER = 0x3,
	PIC_OCW3_POLL = 0x4,
	PIC_OCW3_ENABLE = 0x8,
	PIC_OCW3_SMM = 0x60,
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

	uint8_t icw[4];
	uint8_t ocw[3];
} Pic;


#endif
