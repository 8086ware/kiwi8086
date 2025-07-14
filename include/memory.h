#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "system.h"

uint32_t seg_mem(uint16_t seg, uint16_t offset);

void write_address8(Sys8086* sys, uint32_t address, uint8_t value, _Bool port);
void write_address16(Sys8086* sys, uint32_t address, uint16_t value, _Bool port);
uint8_t read_address8(Sys8086* sys, uint32_t address, _Bool port);
uint16_t read_address16(Sys8086* sys, uint32_t address, _Bool port);

#endif