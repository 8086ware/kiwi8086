#include <stdint.h>
#include <SDL3/SDL.h>

SDL_Color index_cga_palette_0(uint8_t num, _Bool intensity)
{
    SDL_Color color;

    switch(num)
    {
        case 1:
        {
            color.r = 0;
            color.g = 0xAA;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
        case 2:
        {
            color.r = 0xAA;
            color.g = 0;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
        case 3:
        {
            color.r = 0xAA;
            color.g = 0x55;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
    }

    return color;
}

SDL_Color index_cga_palette_1(uint8_t num, _Bool intensity)
{
    SDL_Color color;
    switch(num)
    {
        case 1:
        {
            color.r = 0;
            color.g = 0xAA;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }
        case 2:
        {
            color.r = 0xAA;
            color.g = 0;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }

        case 3:
        {
            color.r = 0xAA;
            color.g = 0xAA;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }
    }

    return color;
}

SDL_Color index_4bit_palette(uint8_t num)
{
    SDL_Color color;

    switch(num)
    {
        case 0: // black
        {
            color.r = 0;
            color.g = 0;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
        case 1: // blue
        {
            color.r = 0;
            color.g = 0;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }
        case 2: // green
        {
            color.r = 0;
            color.g = 0xAA;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
        case 3: // cyan
        {
            color.r = 0;
            color.g = 0xAA;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }
        case 4: // red
        {
            color.r = 0xAA;
            color.g = 0;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
        case 5: // magenta
        {
            color.r = 0xAA;
            color.g = 0;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }
        case 6: // brown
        {
            color.r = 0xAA;
            color.g = 0x55;
            color.b = 0;
            color.a = 0xFF;
            break;
        }
        case 7: // light gray
        {
            color.r = 0xAA;
            color.g = 0xAA;
            color.b = 0xAA;
            color.a = 0xFF;
            break;
        }
        case 8: // dark gray
        {
            color.r = 0x55;
            color.g = 0x55;
            color.b = 0x55;
            color.a = 0xFF;
            break;
        }
        case 9: // light blue
        {
            color.r = 0x55;
            color.g = 0x55;
            color.b = 0xFF;
            color.a = 0xFF;
            break;
        }
        case 10: // light green
        {
            color.r = 0x55;
            color.g = 0xFF;
            color.b = 0x55;
            color.a = 0xFF;
            break;
        }
        case 11: // light cyan
        {
            color.r = 0x55;
            color.g = 0xFF;
            color.b = 0xFF;
            color.a = 0xFF;
            break;
        }
        case 12: // lightred
        {
            color.r = 0xFF;
            color.g = 0x55;
            color.b = 0x55;
            color.a = 0xFF;
            break;
        }
        case 13: // light magenta
        {
            color.r = 0xFF;
            color.g = 0x55;
            color.b = 0xFF;
            color.a = 0xFF;
            break;
        }
        case 14: // light brown
        {
            color.r = 0xFF;
            color.g = 0xFF;
            color.b = 0x55;
            color.a = 0xFF;
            break;
        }
        case 15: // white
        {
            color.r = 0xFF;
            color.g = 0xFF;
            color.b = 0xFF;
            color.a = 0xFF;
            break;
        }
    }        
    
    return color;
}