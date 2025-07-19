#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

void cmp8(Sys8086* sys, uint8_t val1, uint8_t val2);
void cmp16(Sys8086* sys, uint16_t val1, uint16_t val2);

void inc8(Sys8086* sys, uint8_t* value);
void dec8(Sys8086* sys, uint8_t* value);
void inc16(Sys8086* sys, uint16_t* value);
void dec16(Sys8086* sys, uint16_t* value);

void jmp(Sys8086* sys, uint16_t segment, uint16_t offset);

void mov8(Sys8086* sys, uint8_t* dest, uint8_t* src);
void mov16(Sys8086* sys, uint16_t* dest, uint16_t* src);

void mul8(Sys8086* sys, uint8_t value);
void mul16(Sys8086* sys, uint16_t value);

void push(Sys8086* sys, uint16_t value);
void pop(Sys8086* sys, uint16_t* value);

void add8(Sys8086* sys, uint8_t* to, uint8_t added);
void add16(Sys8086* sys, uint16_t* to, uint16_t added);

#endif
