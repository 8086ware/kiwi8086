#ifndef PPI_H
#define PPI_H

#include <stdint.h>

#define PPI_DIP_SWITCH 0x2C

enum Ppi_Port
{
	PPI_PORT_A = 0x60,
	PPI_PORT_B = 0x61,
	PPI_PORT_C = 0x62,
	PPI_PORT_CTRL = 0x63,
};

enum Ppi_Port_Flags
{
	PPI_FLAG_B_TIMER2_GATE = 0x1,
	PPI_FLAG_B_TIMER2_DATA = 0x2,
	PPI_FLAG_B_JUMPER_E5 = 0x4,
	PPI_FLAG_B_READ_SWITCHES = 0x8, // 1 = dip switch 5-8 is in PC0-3, 0 = dip switch 1-4 in PC0-3
	PPI_FLAG_B_KEYBOARD_CLOCK = 0x40,
	PPI_FLAG_B_KEYBOARD_DISABLE = 0x80,

	PPI_FLAG_C_DIP_SWITCH_1 = 0xF,

	PPI_FLAG_CTRL_LOWER_C_INPUT = 0x1,
	PPI_FLAG_CTRL_B_INPUT = 0x2,

	PPI_FLAG_CTRL_HIGHER_C_INPUT = 0x8,
	PPI_FLAG_CTRL_A_INPUT = 0x16,
};

// PPI modes:
// 0 - Simple input or output mode
// In this mode, A and B are 2 simple io ports but C is split into 2 4 bit ports.
// Outputs are latched, inputs are not latched. Ports can't interrupt or handshake
// can only operate in mode 0 for now

// 1 - Input or output with handshake
// handshake signals between peripherals. Ports are simple 2 8 bit ports.
// port C is used for hand shake signals. Specifically 2 (A and B) 3 bit lines. Remaining 2 bit is for i/o operations (simple)
// Input and Output are latched

// 2 - Bidirectional data transfer mode
// Usually used in data transfer between computers.
// Port A is configured as bidirectional, and Port B in mode 1 or 0.
// Port A uses five signals from Port C as handshake signals for data transfer
// Remaining three signals from Port C is simple IO or handshake for port B.

typedef struct Ppi
{
	uint8_t regs[3];
	uint8_t control_byte;
} Ppi;

#endif
