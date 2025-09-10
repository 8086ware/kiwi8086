#include "dma.h"
#include "system.h"

uint8_t handle_dma_port(Sys8086* sys, uint16_t port, uint8_t value, _Bool read)
{
	switch (port)
	{
	case DMA_STATUS_COMMAND_PORT:
	{
		if (read) // status
		{
			return sys->dma.status;
		}

		else // command
		{

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
	}
}

void dma_cycle(Sys8086* sys)
{
	// 5mhz


}