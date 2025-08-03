#ifndef CRTC_H
#define CRTC_H

#include <stdint.h>

enum CRTC_Port
{
    CRTC_INDEX_REGISTER = 0x3D4,
    CRTC_DATA_REGISTER = 0x3D5,
};

typedef struct Crt_controller
{
	uint16_t cursor_address;
} Crt_controller;

#endif