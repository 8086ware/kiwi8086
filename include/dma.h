#ifndef DMA_H
#define DMA_H

// The DMA Controller is a circuit for the 8088/8086 system
// It allows external devices to directly transfer information/from the system memory
// It bypasses the CPU. Memory to memory transfer is also provided.
// The 8237 offers a wide variety of programmable control features.
// 5 mhz

#include <stdint.h>

enum DMA_Flags
{
	DMA_COMMAND_FLAG_MEM2MEM = 0x1,
	DMA_COMMAND_FLAG_CHANNEL_0_ADDRESS_HOLD = 0x2,
	DMA_COMMAND_FLAG_CONTROLLER_DISABLE = 0x4,
	DMA_COMMAND_FLAG_PRIORITY_ROTATE = 0x10,

	DMA_MODE_FLAG_SELECT_CHANNEL = 0x3,
	DMA_MODE_FLAG_TRANSFER_MODE = 0xC,
	DMA_MODE_FLAG_AUTOINITALIZATION = 0x10,
	DMA_MODE_FLAG_DECREMENT_SELECT = 0x20,
	DMA_MODE_FLAG_SELECT_MODE = 0xC0,

	DMA_REQUEST_FLAG_SELECT_CHANNEL = 0x3,
	DMA_REQUEST_FLAG_SET_REQUEST = 0x4,

	DMA_STATUS_FLAG_TC_CHANNEL_0 = 0x1,
	DMA_STATUS_FLAG_TC_CHANNEL_1 = 0x2,
	DMA_STATUS_FLAG_TC_CHANNEL_2 = 0x4,
	DMA_STATUS_FLAG_TC_CHANNEL_3 = 0x8,

	DMA_STATUS_FLAG_REQUEST_CHANNEL_0 = 0x10,
	DMA_STATUS_FLAG_REQUEST_CHANNEL_1 = 0x20,
	DMA_STATUS_FLAG_REQUEST_CHANNEL_2 = 0x40,
	DMA_STATUS_FLAG_REQUEST_CHANNEL_3 = 0x80,

	DMA_SINGLE_MASK_FLAG_SELECT_CHANNEL = 0x3,
	DMA_SINGLE_MASK_SET_MASK = 0x4,

	DMA_MULTI_MASK_SET_CHANNEL_0 = 0x1,
	DMA_MULTI_MASK_SET_CHANNEL_1 = 0x2,
	DMA_MULTI_MASK_SET_CHANNEL_2 = 0x4,
	DMA_MULTI_MASK_SET_CHANNEL_3 = 0x8,
};

enum DMA_Port
{
	DMA_CHANNEL_0_ADDRESS_PORT = 0x0,
	DMA_CHANNEL_0_WORD_COUNT_PORT = 0x1,
	DMA_CHANNEL_1_ADDRESS_PORT = 0x2,
	DMA_CHANNEL_1_WORD_COUNT_PORT = 0x3,
	DMA_CHANNEL_2_ADDRESS_PORT = 0x4,
	DMA_CHANNEL_2_WORD_COUNT_PORT = 0x5,
	DMA_CHANNEL_3_ADDRESS_PORT = 0x6,
	DMA_CHANNEL_3_WORD_COUNT_PORT = 0x7,
	DMA_STATUS_COMMAND_PORT = 0x8, // R = Status, W = Command
	DMA_REQUEST_PORT = 0x9,
	DMA_MASK_PORT = 0xA,
	DMA_MODE_REGISTER = 0xB,
	DMA_CLEAR_FLIP_FLOP = 0xC,
	DMA_MASTER_CLEAR_TEMP_PORT = 0xD,
	DMA_CLEAR_MASK_REGISTER = 0xE,
	DMA_MULTIPLE_MASK_REGISTER = 0xF,

	DMA_CHANNEL_1_HIGH_ORDER_4_BITS = 0x81,
	DMA_CHANNEL_2_HIGH_ORDER_4_BITS = 0x82,
	DMA_CHANNEL_3_HIGH_ORDER_4_BITS = 0x83,
};

typedef struct DMA_Channel
{
	uint16_t current_address; // Is incremented each transfer
	uint16_t current_word; // Is decremented each transfer
	uint16_t base_address; // Original current_address
	uint16_t base_word; // Original current_word
} DMA_Channel;

typedef struct DMA
{
	uint8_t status;

	_Bool flip_flop;

	uint8_t dreq;
	uint8_t dmask;
	uint8_t dack;

	DMA_Channel channels[4];
} DMA;

#endif