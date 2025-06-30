#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_DISPLAY_PAGES 8

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

	uint8_t cursor_x[VGA_DISPLAY_PAGES];
	uint8_t cursor_y[VGA_DISPLAY_PAGES];

	int display_page;
} VGA;

#endif
