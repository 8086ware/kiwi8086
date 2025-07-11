#include <stdint.h>

uint32_t seg_mem(uint16_t seg, uint16_t offset)
{
	return seg * 0x10 + offset;
}