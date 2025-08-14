#ifndef CGA_H
#define CGA_H

#define CGA_ADDRESS 0xB8000
#define CGA_RAM_SIZE 0x4000 // 16k of ram

#include <stdint.h>

enum CGA_Mode_Control_Flags
{
	CGA_MODE_CONTROL_HIGH_RES = 0x1, // 1 = 80 columns (160 bytes per row), 0 = 40 columns (80 bytes per row). Reprogram CRT controller accordingly
	CGA_MODE_CONTROL_GRAPHICS = 0x2, // 1 = Display ram treated as graphics, 0 = Display ram is text (4 pages (80x25) or 8 pages (40x25))
	CGA_MODE_CONTROL_BLACK_AND_WHITE = 0x4, // 1 = Turns on black and white. 0 = Color
	CGA_MODE_CONTROL_VIDEO = 0x8, // Turns screen on/off (1/0)
	CGA_MODE_CONTROL_HIGH_RES_GRAPHICS = 0x10, // On = 640px (2 colors) off = 320px (4 colors) graphics
	CGA_MODE_CONTROL_BLINKING = 0x20, // 1 = Attribute bit 7 set will blink. Otherwise they will have high intensity backgrounds.
};

enum CGA_Color_Control_Flags
{
	CGA_COLOR_CONTROL_BORDER_BACKGROUND_FOREGROUND = 0x7, // bits 0-3. Text mode = color overscan. 320x200 mode = background and border. 640x200 = foreground color
	CGA_COLOR_CONTROL_BRIGHT_FOREGROUND = 0x10, // 1 = high intensity, 0 = normal 320x200 only
	CGA_COLOR_CONTROL_PALETTE = 0x20, // 0 = red green yellow palette 1 = magenta cyan and white (including black as well for both)
};

enum CGA_Port
{
	CGA_MODE_CONTROL_REGISTER_PORT = 0x3D8,
	CGA_STATUS_REGISTER_PORT = 0x3DA,
	CGA_COLOR_CONTROL_REGISTER_PORT = 0x3D9,
};

typedef struct CGA 
{
	uint8_t mode_ctrl_reg;
	uint8_t color_ctrl_reg;

	uint8_t ram[CGA_RAM_SIZE];
} CGA;

#endif
