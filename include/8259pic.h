#ifndef PIC8259_H
#define PIC8259_H

enum pic8259_port
{
	PIC_MASTER_COMMAND = 0x0020,
	PIC_MASTER_DATA    = 0x0021,
	PIC_SLAVE_COMMAND  = 0x00A0,
	PIC_SLAVE_DATA     = 0x00A1,
};

typedef struct pic8259
{	
	uint8_t command;
	// -1 if no interrupt, > -1 contains interrupt number
	int irq;
	uint8_t imr; // interrupt mask register, will prevent any irq at x bit set
	uint16_t vector_offset; // offset from 0000:0000, or the ivt
} pic8259;
#endif