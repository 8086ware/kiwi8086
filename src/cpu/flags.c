#include "system.h"

void cpu_modify_flag_carry(CPU* cpu, uint16_t old_val, uint16_t new_val, _Bool word)
{
	uint8_t old_val8 = old_val;
	uint8_t new_val8 = new_val;

	if (word)
	{
		for (int i = 0; i <= 16; i++)
		{
			if (((old_val << i) & 0b1000000000000000) == 0b1000000000000000)
			{
				cpu->flag.whole &= ~FLAG_CARRY;
				break;
			}

			if (((new_val << i) & 0b1000000000000000) == 0b1000000000000000)
			{
				cpu->flag.whole |= FLAG_CARRY;
				break;
			}
		}
	}

	else
	{
		for (int i = 0; i <= 8; i++)
		{
			if (((old_val8 << i) & 0b10000000) == 0b10000000)
			{
				cpu->flag.whole &= ~FLAG_CARRY;
				break;
			}

			if (((new_val8 << i) & 0b10000000) == 0b10000000)
			{
				cpu->flag.whole |= FLAG_CARRY;
				break;
			}
		}
	}
}

// Sees if amount of bits set is even (on) or odd (off)
// only least significant byte so its fine passing 16 bit value

void cpu_modify_flag_parity(CPU* cpu, uint8_t val)
{
	int bit_amount = 0;

	for (int i = 0; i <= 8; i++)
	{
		if (((val << i) & 0b10000000) == 0b10000000)
		{
			bit_amount++;
		}
	}

	if (bit_amount % 2 == 0)
	{
		cpu->flag.whole |= FLAG_PARITY;
	}

	else
	{
		cpu->flag.whole &= ~FLAG_PARITY;
	}
}

// Sees if there is a new bit set in the 4th bit of a value (lsb on 16 bit values, so it works)

void cpu_modify_flag_half_carry(CPU* cpu, uint8_t old_val, uint8_t new_val)
{
	if ((old_val & 0b00001000) == 0b00001000)
	{
		cpu->flag.whole &= ~FLAG_HALF_CARRY;
	}

	else if ((new_val & 0b00001000) == 0b00001000)
	{
		cpu->flag.whole |= FLAG_HALF_CARRY;
	}
}

// Sees if the new value is 0 (works on 8 bit and 16 bit)

void cpu_modify_flag_zero(CPU* cpu, uint16_t val)
{
	if (val == 0)
	{
		cpu->flag.whole |= FLAG_ZERO;
	}

	else
	{
		cpu->flag.whole &= ~FLAG_ZERO;
	}
}

void cpu_modify_flag_sign(CPU* cpu, uint16_t val, _Bool word)
{
	uint8_t val8 = val;

	if (word)
	{
		if ((val & 0b1000000000000000) == 0b1000000000000000)
		{
			cpu->flag.whole |= FLAG_SIGN;
		}

		else
		{
			cpu->flag.whole &= ~FLAG_SIGN;
		}
	}

	else
	{
		if ((val8 & 0b10000000) == 0b10000000)
		{
			cpu->flag.whole |= FLAG_SIGN;
		}

		else
		{
			cpu->flag.whole &= ~FLAG_SIGN;
		}
	}
}

// assume subtraction (like cmp) between op1 and op2, for addition change the operands as needed
void cpu_modify_flag_overflow(CPU* cpu, int16_t op1, int16_t op2, int16_t result, _Bool word)
{
	_Bool op1_neg = 0;
	_Bool op2_neg = 0;
	_Bool result_neg = 0;

	if (word)
	{
		op1_neg = op1 & 0b1000000000000000;
		op2_neg = op2 & 0b1000000000000000;
		result_neg = result & 0b1000000000000000;
	}

	else
	{
		int8_t op1_8 = op1;
		int8_t op2_8 = op2;
		int8_t result_8 = result;

		op1_neg = op1_8 & 0b10000000;
		op2_neg = op2_8 & 0b10000000;
		result_neg = result_8 & 0b10000000;
	}

	// -127 to 127 (example)
	// -127 - 127 = 2
	// op1_neg - op2_neg

	if (op1_neg && !op2_neg && !result_neg)
	{
		cpu->flag.whole |= FLAG_OVERFLOW;
	}

	else if (!op1_neg && op2_neg && result_neg)
	{
		cpu->flag.whole |= FLAG_OVERFLOW;
	}

	else
	{
		cpu->flag.whole &= ~FLAG_OVERFLOW;
	}
}
