#include "dma.h"
#include "system.h"
#include "memory.h"

uint8_t handle_dma_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	switch (port)
	{
	case DMA_STATUS_COMMAND_PORT:
	{
		if (read) // status
		{
			sys->dma.status |= (sys->dma.dreq << 4);
			return sys->dma.status;
		}

		else // command
		{

		}
		break;
	}
	case DMA_CHANNEL_1_HIGH_ORDER_4_BITS:
	case DMA_CHANNEL_2_HIGH_ORDER_4_BITS:
	case DMA_CHANNEL_3_HIGH_ORDER_4_BITS:
	{
		if (!read)
		{
			uint8_t reg = (port - 0x80);

			sys->dma.channels[reg].base_address |= ((uint32_t)value << 16);
		}

		break;
	}
	case DMA_CHANNEL_0_ADDRESS_PORT:
	case DMA_CHANNEL_1_ADDRESS_PORT:
	case DMA_CHANNEL_2_ADDRESS_PORT:
	case DMA_CHANNEL_3_ADDRESS_PORT:
	{
		uint8_t reg = (port - DMA_CHANNEL_0_ADDRESS_PORT) / 2;

		if (read)
		{
			if (!sys->dma.flip_flop) // low byte
			{
				sys->dma.flip_flop = 1;
				return (uint8_t)(sys->dma.channels[reg].current_address & 0x00ff);
			}

			else // high byte
			{
				sys->dma.flip_flop = 0;
				return (uint8_t)(sys->dma.channels[reg].current_address >> 8);
			}
		}

		else
		{
			if (!sys->dma.flip_flop) // low byte
			{
				sys->dma.channels[reg].base_address &= ~0x00ff;
				sys->dma.channels[reg].current_address &= ~0x00ff;
				sys->dma.channels[reg].base_address |= (uint16_t)value;
				sys->dma.channels[reg].current_address |= (uint16_t)value;
				sys->dma.flip_flop = 1;
			}

			else // high byte
			{
				sys->dma.channels[reg].base_address &= ~0xff00;
				sys->dma.channels[reg].current_address &= ~0xff00;
				sys->dma.channels[reg].base_address |= ((uint16_t)value << 8);
				sys->dma.channels[reg].current_address |= ((uint16_t)value << 8);
				sys->dma.flip_flop = 0;
			}
		}

		break;
	}
	case DMA_CHANNEL_0_WORD_COUNT_PORT:
	case DMA_CHANNEL_1_WORD_COUNT_PORT:
	case DMA_CHANNEL_2_WORD_COUNT_PORT:
	case DMA_CHANNEL_3_WORD_COUNT_PORT:
	{
		uint8_t reg = ((port - DMA_CHANNEL_0_WORD_COUNT_PORT) / 2);

		if (read)
		{
			if (!sys->dma.flip_flop) // low byte
			{
				sys->dma.flip_flop = 1;
				return (uint8_t)(sys->dma.channels[reg].current_word & 0x00ff);
			}

			else // high byte
			{
				sys->dma.flip_flop = 0;
				return (uint8_t)(sys->dma.channels[reg].current_word >> 8);
			}
		}

		else
		{
			if (!sys->dma.flip_flop) // low byte
			{
				sys->dma.channels[reg].base_word &= ~0x00ff;
				sys->dma.channels[reg].current_word &= ~0x00ff;
				sys->dma.channels[reg].base_word |= (uint16_t)value;
				sys->dma.channels[reg].current_word |= (uint16_t)value;
				sys->dma.flip_flop = 1;
			}

			else // high byte
			{
				sys->dma.channels[reg].base_word &= ~0xff00;
				sys->dma.channels[reg].current_word &= ~0xff00;
				sys->dma.channels[reg].base_word |= ((uint16_t)value << 8);
				sys->dma.channels[reg].current_word |= ((uint16_t)value << 8);
				sys->dma.flip_flop = 0;
			}
		}

		break;
	}
	case DMA_REQUEST_PORT:
	{
		if(!read)
		{
			int channel = value & DMA_REQUEST_FLAG_SELECT_CHANNEL;

			if(value & DMA_REQUEST_FLAG_SET_REQUEST)
			{
				sys->dma.dreq |= (1 << (channel));
			}

			else
			{
				sys->dma.dreq &= ~(1 << (channel));
			}

		}
		break;
	}
	case DMA_MASK_PORT:
	{
		if(!read)
		{
			int channel = value & DMA_SINGLE_MASK_FLAG_SELECT_CHANNEL;

			if(value & DMA_SINGLE_MASK_SET_MASK)
			{
				sys->dma.dmask |= (1 << (channel));
			}

			else
			{
				sys->dma.dmask &= ~(1 << (channel));
			}

		}
		break;
	}
	case DMA_MODE_PORT:
	{
		if (!read)
		{
			int channel = value & DMA_MODE_FLAG_SELECT_CHANNEL;
			sys->dma.channels[channel].mode = value;
		}
		break;
	}
	case DMA_MASTER_CLEAR_TEMP_PORT:
	{
		if(!read)
		{
			sys->dma.flip_flop = 0;
			sys->dma.status = 0;
			sys->dma.dmask = 0xF;
		}
		break;
	}
	case DMA_CLEAR_FLIP_FLOP_PORT:
	{
		if(!read)
		{
			sys->dma.flip_flop = 0;
		}
		break;
	}
	case DMA_CLEAR_MASK_PORT:
	{
		if(!read)
		{
			sys->dma.dmask = 0;
		}
		break;
	}
	case DMA_MULTIPLE_MASK_PORT:
	{
		if (!read)
		{
			sys->dma.dmask = value & 0xf;
		}
	}
}
}

void dma_cycle(Sys8086* sys)
{
	// 5mhz


}
