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

enum CPU_Group_Opcode
{
	GROUP_OPCODE_80 = 0x80,
	GROUP_OPCODE_81 = 0x81,
	GROUP_OPCODE_83 = 0x83,
	GROUP_OPCODE_D0 = 0xD0,
	GROUP_OPCODE_D1 = 0xD1,
	GROUP_OPCODE_D2 = 0xD2,
	GROUP_OPCODE_D3 = 0xD3,
	GROUP_OPCODE_F6 = 0xF6,
	GROUP_OPCODE_F7 = 0xF7,
	GROUP_OPCODE_FE = 0xFE,
	GROUP_OPCODE_FF = 0xFF,
};

enum CPU_Opcode_Prefix
{
	PREFIX_ES = 0x26,
	PREFIX_CS = 0x2E,
	PREFIX_SS = 0x36,
	PREFIX_DS = 0x3E,
};


enum CPU_Opcode
{
	ADD_RM8_IMM8 = 0x0,// Opcode group 80
	ADD_RM16_IMM16 = 0x0,// Opcode group 81
	ADD_RM16_IMM8 = 0x0, // Opcode group 83
	ADD_RM8_R8 = 0x00,
	ADD_RM16_R16 = 0x01,
	ADD_R8_RM8 = 0x02,
	ADD_R16_RM16 = 0x03,
	ADD_AL_IMM8 = 0x04,
	ADD_AX_IMM16 = 0x05,

	AND_AL_IMM8 = 0x24,
	AND_AX_IMM16 = 0x25,
	AND_RM8_IMM8 = 0x4, // Opcode group 80
	AND_RM16_IMM16 = 0x4, // Opcode group 81
	AND_RM16_IMM8 = 0x4, // Opcode group 83
	AND_RM8_R8 = 0x20,
	AND_RM16_R16 = 0x21,
	AND_R8_RM8 = 0x22,
	AND_R16_RM16 = 0x23,

	CALL_REL16 = 0xE8,
	CALL_RM16 = 0x2, // Opcode group FF
	CALL_PTR16_16 = 0x9A,
	CALL_M16_16 = 0x3, // Opcode group FF

	CMP_AL_IMM8 = 0x3C,
	CMP_AX_IMM16 = 0x3D,
	CMP_RM8_IMM8 = 0x7, // Opcode group 80
	CMP_RM16_IMM16 = 0x7, // Opcode group 81
	CMP_RM16_IMM8 = 0x7, // Opcode group 83
	CMP_RM8_R8 = 0x38,
	CMP_RM16_R16 = 0x39,
	CMP_R8_RM8 = 0x3A,
	CMP_R16_RM16 = 0x3B,

	// Affects flag register
	DEC_RM8 = 0x1, // Opcode group FE
	DEC_RM16 = 0x1, // Opcode group FF
	DEC_R16 = 0x48,

	HLT = 0xF4,

	IN_AL_IMM8 = 0xE4,
	IN_AX_IMM8 = 0xE5,
	IN_AL_DX = 0xEC,
	IN_AX_DX = 0xED,

	// Affects flag register
	INC_RM8 = 0x0, // Opcode group FE
	INC_RM16 = 0x0, // Opcode group FF
	INC_R16 = 0x40,

	INT_IMM8 = 0xCD,

	IRET = 0xCF,

	// Affects flag register
	JMP_REL8 = 0xEB,
	JMP_REL16 = 0xE9,
	JMP_RM16 = 0x4, // Opcode group FF
	JMP_PTR16_16 = 0xEA,

	JA_REL8 = 0x77,
	JAE_REL8 = 0x73,
	JB_REL8 = 0x72,
	JBE_REL8 = 0x76,
	JCXZ_REL8 = 0xE3,
	JE_REL8 = 0x74,
	JG_REL8 = 0x7F,
	JGE_REL8 = 0x7D,
	JL_REL8 = 0x7C,
	JLE_REL8 = 0x7E,
	JNE_REL8 = 0x75,
	JNO_REL8 = 0x71,
	JNP_REL8 = 0x7B,
	JNS_REL8 = 0x79,
	JO_REL8 = 0x70,
	JP_REL8 = 0x7A,
	JS_REL8 = 0x78,

	MOV_RM8_R8 = 0x88,
	MOV_RM16_R16 = 0x89,
	MOV_R8_RM8 = 0x8A,
	MOV_R16_RM16 = 0x8B,
	MOV_RM16_SREG = 0x8C,
	MOV_SREG_RM16 = 0x8E,
	MOV_AL_MOFFS8 = 0xA0,
	MOV_AX_MOFFS16 = 0xA1,
	MOV_MOFFS8_AL = 0xA2,
	MOV_MOFFS16_AX = 0xA3,
	MOV_R8_IMM8 = 0xB0,
	MOV_R16_IMM16 = 0xB8,
	MOV_RM8_IMM8 = 0xC6,
	MOV_RM16_IMM16 = 0xC7,

	MUL_RM8 = 0x4, // Opcode group F6
	MUL_RM16 = 0x4, // Opcode group F7

	OR_AL_IMM8 = 0x0C,
	OR_AX_IMM16 = 0x0D,
	OR_RM8_IMM8 = 0x1, // Opcode group 80
	OR_RM16_IMM16 = 0x1, // Opcode group 81
	OR_RM16_IMM8 = 0x1, // Opcode group 83
	OR_RM8_R8 = 0x08,
	OR_RM16_R16 = 0x09,
	OR_R8_RM8 = 0x0A,
	OR_R16_RM16 = 0x0B,

	OUT_IMM8_AL = 0xE6,
	OUT_IMM8_AX = 0xE7,
	OUT_DX_AL = 0xEE,
	OUT_DX_AX = 0xEF,

	PUSH_R16 = 0x50,
	PUSH_RM16 = 0x6, // Opcode group FF
	PUSH_SREG = 0x6,

	POP_R16 = 0x58,
	POP_RM16 = 0x8F,
	POP_SREG = 0x07,

	RET_NEAR = 0xC3,
	RET_FAR = 0xCB,
	RET_NEAR_IMM16 = 0xC2,
	RET_FAR_IMM16 = 0xCA,

	// Same as SHL
	SAL_RM8_1 = 0x4, // Opcode group D0
	SAL_RM8_CL = 0x4, // Opcode group D2
	SAL_RM16_1 = 0x4, // Opcode group D1
	SAL_RM16_CL = 0x4, // Opcode group D3

	SAR_RM8_1 = 0x7, // Opcode group D0
	SAR_RM8_CL = 0x7, // Opcode group D2
	SAR_RM16_1 = 0x7, // Opcode group D1
	SAR_RM16_CL = 0x7, // Opcode group D3

	SHR_RM8_1 = 0x5, // Opcode group D0
	SHR_RM8_CL = 0x5, // Opcode group D2
	SHR_RM16_1 = 0x5, // Opcode group D1
	SHR_RM16_CL = 0x5, // Opcode group D3

	SUB_AL_IMM8 = 0x2C,
	SUB_AX_IMM16 = 0x2D,
	SUB_RM8_IMM8 = 0x5, // Opcode group 80
	SUB_RM16_IMM16 = 0x5, // Opcode group 81
	SUB_RM16_IMM8 = 0x5, // Opcode group 83
	SUB_RM8_R8 = 0x28,
	SUB_RM16_R16 = 0x29,
	SUB_R8_RM8 = 0x2A,
	SUB_R16_RM16 = 0x2B,

	TEST_AL_IMM8 = 0xA8,
	TEST_AX_IMM16 = 0xA9,
	TEST_RM8_IMM8 = 0x0, // Opcode group F6
	TEST_RM16_IMM16 = 0x0, // Opcode group F7
	TEST_RM8_R8 = 0x84,
	TEST_RM16_R16 = 0x85,

	XCHG_AX_R16 = 0x90,
	XCHG_RM8_R8 = 0x86,
	XCHG_RM16_R16 = 0x87,

	XOR_AL_IMM8 = 0x34,
	XOR_AX_IMM16 = 0x35,
	XOR_RM8_IMM8 = 0x6, // Opcode group 80
	XOR_RM16_IMM16 = 0x6, // Opcode group 81
	XOR_RM16_IMM8 = 0x6, // Opcode group 83
	XOR_RM8_R8 = 0x30,
	XOR_RM16_R16 = 0x31,
	XOR_R8_RM8 = 0x32,
	XOR_R16_RM16 = 0x33,
};

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
	_Bool prev_byte_success;
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
