#ifndef CRTC_H
#define CRTC_H

#include <stdint.h>

enum CRTC_Port
{
    CRTC_INDEX_REGISTER_PORT = 0x3D4,
    CRTC_DATA_REGISTER_PORT = 0x3D5,
};

enum CRTC_Data_Reg_Options
{
	CRTC_DATA_CURSOR_START_SCANLINE = 10,
	CRTC_DATA_CURSOR_END_SCANLINE = 11,
	CRTC_DATA_START_ADDRESS_HIGH = 12,
	CRTC_DATA_START_ADDRESS_LOW = 13,
	CRTC_DATA_CURSOR_ADDRESS_HIGH = 14,
	CRTC_DATA_CURSOR_ADDRESS_LOW = 15,
};

typedef struct Crt_controller
{
	uint8_t index;

	uint8_t cursor_end_scan_line;
	uint8_t cursor_start_scan_line;
	uint16_t start_address;
	uint16_t cursor_address;
} Crt_controller;

#endif
