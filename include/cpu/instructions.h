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

void sub8(Sys8086* sys, uint8_t* to, uint8_t subbed);
void sub16(Sys8086* sys, uint16_t* to, uint16_t subbed);

void sal8(Sys8086* sys, int8_t* value, uint8_t amount);
void sar8(Sys8086* sys, int8_t* value, uint8_t amount);
void shr8(Sys8086* sys, uint8_t* value, uint8_t amount);

void sal16(Sys8086* sys, int16_t* value, uint8_t amount);
void sar16(Sys8086* sys, int16_t* value, uint8_t amount);
void shr16(Sys8086* sys, uint16_t* value, uint8_t amount);

void or8(Sys8086* sys, uint8_t* value, int8_t or);
void or16(Sys8086* sys, uint16_t* value, int16_t or);

void and8(Sys8086* sys, uint8_t* value, uint8_t and);
void and16(Sys8086* sys, uint16_t* value, uint16_t and);



#endif

