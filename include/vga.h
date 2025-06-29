#ifndef VGA_H
#define VGA_H

#include <stdint.h>

typedef struct VGA_textmode_cell 
{
	unsigned char disp;
	uint8_t attrib;
} VGA_textmode_cell;

typedef struct VGA
{
	int columns;
	int rows;

	VGA_textmode_cell* mem;
} VGA;

#endif
