#ifndef CPU_H
#define CPU_H

#include <stdint.h>

//IMM8/16 = An value given, a constant
//R/M/8/16 = Either a register or memory place that has a 16/8 bit value
//MOFFS16/8 = A memory place that has an 16/8bit value
//REG8/16 = A 16/8 bit register
//PTR16 = 16 bit segment (far jump/call)

#define GROUP_OPCODE_FF 0xFF
#define GROUP_OPCODE_FE 0xFE

#define PREFIX_ES 0x26
#define PREFIX_CS 0x2E
#define PREFIX_SS 0x36
#define PREFIX_DS 0x3E

#define ADD_RM8_R8 0x00
#define ADD_AL_IMM8 0x04
#define ADD_AX_IMM16 0x05

#define CALL_REL16 0xE8
#define CALL_RM16 0x2 // OPCODE GROUP FF
#define CALL_PTR16_16 0x9A
#define CALL_M16_16 0x3 // OPCODE GROUP FF

#define MOV_RM8_R8 0x88
#define MOV_RM16_R16 0x89
#define MOV_R8_RM8 0x8A
#define MOV_R16_RM16 0x8B
#define MOV_RM16_SREG 0x8C
#define MOV_SREG_RM16 0x8E
#define MOV_AL_MOFFS8 0xA0
#define MOV_AX_MOFFS16 0xA1
#define MOV_MOFFS8_AL 0xA2
#define MOV_MOFFS16_AX 0xA3
#define MOV_R8_IMM8 0xB0
#define MOV_R16_IMM16 0xB8
#define MOV_RM8_IMM8 0xC6
#define MOV_RM16_IMM16 0xC7

#define INC_RM8 0x0 // Opcode group FE
#define INC_RM16 0x0 // Opcode group FF
#define INC_R16 0x40

#define JMP_REL8 0xEB
#define JMP_REL16 0xE9
#define JMP_RM16 0x4 // Opcode group FF
#define JMP_PTR16_16 0xEA

#define PUSH_R16 0x50
#define PUSH_RM16 0x6 // Opcode group FF
#define PUSH_SREG 0x06

#define POP_R16 0x58
#define POP_RM16 0x8F
#define POP_SREG 0x07

#define INT_IMM8 0xCD

typedef union Register
{
	struct
	{
		uint8_t low;
		uint8_t high;
	};

	uint16_t whole;
} Register;

typedef struct CPU
{
	// General purpose registers

	Register ax;
	Register bx;
	Register cx;
	Register dx;

	// Segment registers

	Register cs;
	Register ds;
	Register es;
	Register ss;

	// Stack pointer

	Register sp;

	// Base pointer

	Register bp;

	// String registers

	Register si;
	Register di;

	// Instruction pointer

	Register ip;

	// Status flag

	Register flag;
} CPU;


#endif