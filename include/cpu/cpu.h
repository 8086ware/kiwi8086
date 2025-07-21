#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define FLAG_CARRY      0b0000000000000001
#define FLAG_PARITY     0b0000000000000100
#define FLAG_HALF_CARRY 0b0000000000010000
#define FLAG_ZERO       0b0000000001000000
#define FLAG_SIGN       0b0000000010000000
#define FLAG_OVERFLOW   0b0000100000000000

//IMM8/16 = An value given, a constant
//R/M/8/16 = Either a register or memory place that has a 16/8 bit value
//MOFFS16/8 = A memory place that has an 16/8bit value (in current data segment)
//REG8/16 = A 16/8 bit register
//PTR16 = 16 bit segment (far jump/call)

#define GROUP_OPCODE_80 0x80
#define GROUP_OPCODE_81 0x81
#define GROUP_OPCODE_83 0x83
#define GROUP_OPCODE_D0 0xD0
#define GROUP_OPCODE_D1 0xD1
#define GROUP_OPCODE_D2 0xD2
#define GROUP_OPCODE_D3 0xD3
#define GROUP_OPCODE_F6 0xF6
#define GROUP_OPCODE_F7 0xF7
#define GROUP_OPCODE_FE 0xFE
#define GROUP_OPCODE_FF 0xFF

#define PREFIX_ES 0x26
#define PREFIX_CS 0x2E
#define PREFIX_SS 0x36
#define PREFIX_DS 0x3E

// Affects flag register
#define ADD_RM8_IMM8 0x0 // Opcode group 80
#define ADD_RM16_IMM16 0x0 // Opcode group 81
#define ADD_RM16_IMM8 0x0 // Opcode group 83
#define ADD_RM8_R8 0x00
#define ADD_RM16_R16 0x01
#define	ADD_R8_RM8 0x02
#define ADD_R16_RM16 0x03
#define ADD_AL_IMM8 0x04
#define ADD_AX_IMM16 0x05

#define CALL_REL16 0xE8
#define CALL_RM16 0x2 // Opcode group FF
#define CALL_PTR16_16 0x9A
#define CALL_M16_16 0x3 // Opcode group FF

#define CMP_AL_IMM8 0x3C
#define CMP_AX_IMM16 0x3D
#define CMP_RM8_IMM8 0x7 // Opcode group 80
#define CMP_RM16_IMM16 0x7 // Opcode group 81
#define CMP_RM16_IMM8 0x7 // Opcode group 83
#define CMP_RM8_R8 0x38
#define CMP_RM16_R16 0x39
#define CMP_R8_RM8 0x3A
#define CMP_R16_RM16 0x3B

// Affects flag register
#define DEC_RM8 0x1 // Opcode group FE
#define DEC_RM16 0x1 // Opcode group FF
#define DEC_R16 0x48

#define HLT 0xF4

#define IN_AL_IMM8 0xE4
#define IN_AX_IMM8 0xE5
#define IN_AL_DX 0xEC
#define IN_AX_DX 0xED

// Affects flag register
#define INC_RM8 0x0 // Opcode group FE
#define INC_RM16 0x0 // Opcode group FF
#define INC_R16 0x40

#define INT_IMM8 0xCD

#define IRET 0xCF

// Affects flag register
#define JMP_REL8 0xEB
#define JMP_REL16 0xE9
#define JMP_RM16 0x4 // Opcode group FF
#define JMP_PTR16_16 0xEA

#define JA_REL8 0x77
#define JAE_REL8 0x73
#define JB_REL8 0x72
#define JBE_REL8 0x76
#define JCXZ_REL8 0xE3
#define JE_REL8 0x74
#define JG_REL8 0x7F
#define JGE_REL8 0x7D
#define JL_REL8 0x7C
#define JLE_REL8 0x7E
#define JNE_REL8 0x75
#define JNO_REL8 0x71
#define JNP_REL8 0x7B
#define JNS_REL8 0x79
#define JO_REL8 0x70
#define JP_REL8 0x7A
#define JS_REL8 0x78

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

#define MUL_RM8 0x4 // Opcode group F6
#define MUL_RM16 0x4 // Opcode group F7

#define PUSH_R16 0x50
#define PUSH_RM16 0x6 // Opcode group FF
#define PUSH_SREG 0x06

#define POP_R16 0x58
#define POP_RM16 0x8F
#define POP_SREG 0x07

#define RET_NEAR 0xC3
#define RET_FAR 0xCB
#define RET_NEAR_IMM16 0xC2
#define RET_FAR_IMM16 0xCA

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
	_Bool halted;

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

uint16_t* segment_reg_index(CPU* cpu, int index);
uint16_t* reg16_index(CPU* cpu, int index);
uint8_t* reg8_index(CPU* cpu, int index);

void cpu_modify_flag_carry(CPU* cpu, uint16_t old_val, uint16_t new_val, _Bool word);
void cpu_modify_flag_parity(CPU* cpu, uint8_t val);
void cpu_modify_flag_half_carry(CPU* cpu, uint8_t old_val, uint8_t new_val);
void cpu_modify_flag_zero(CPU* cpu, uint16_t val);
void cpu_modify_flag_sign(CPU* cpu, uint16_t val, _Bool word);
void cpu_modify_flag_overflow(CPU* cpu, int16_t op1, int16_t op2, int16_t result, _Bool word);

#endif
