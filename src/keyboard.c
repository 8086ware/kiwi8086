#include "system.h"
#include <SDL3/SDL_events.h>
#include "memory.h"

void poll_keyboard(Sys8086* sys, SDL_Event event)
{
    uint8_t keyboard_scancode = 0;
    
    if(event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN)
    {
        static _Bool two_byte = 0;
        static SDL_Scancode scan_code = 0;
    
        switch(event.key.scancode)
        {
        case SDL_SCANCODE_A:
        {
            keyboard_scancode = 0x1E;
            break;
        }
        case SDL_SCANCODE_B:
        {
            keyboard_scancode = 0x30;
            break;
        }
        case SDL_SCANCODE_C:
        {
            keyboard_scancode = 0x2E;
            break;
        }
        case SDL_SCANCODE_D:
        {
            keyboard_scancode = 0x20;
            break;
        }
        case SDL_SCANCODE_E:
        {
            keyboard_scancode = 0x12;
            break;
        }
        case SDL_SCANCODE_F:
        {
            keyboard_scancode = 0x21;
            break;
        }
        case SDL_SCANCODE_G:
        {
            keyboard_scancode = 0x22;
            break;
        }
        case SDL_SCANCODE_H:
        {
            keyboard_scancode = 0x23;
            break;
        }
        case SDL_SCANCODE_I:
        {
            keyboard_scancode = 0x17;
            break;
        }
        case SDL_SCANCODE_J:
        {
            keyboard_scancode = 0x24;
            break;
        }
        case SDL_SCANCODE_K:
        {
            keyboard_scancode = 0x25;
            break;
        }
        case SDL_SCANCODE_L:
        {
            keyboard_scancode = 0x26;
            break;
        }
        case SDL_SCANCODE_M:
        {
            keyboard_scancode = 0x32;
            break;
        }
        case SDL_SCANCODE_N:
        {
            keyboard_scancode = 0x31;
            break;
        }
        case SDL_SCANCODE_O:
        {
            keyboard_scancode = 0x18;
            break;
        }
        case SDL_SCANCODE_P:
        {
            keyboard_scancode = 0x19;
            break;
        }
        case SDL_SCANCODE_Q:
        {
            keyboard_scancode = 0x10;
            break;
        }
        case SDL_SCANCODE_R:
        {
            keyboard_scancode = 0x13;
            break;
        }
        case SDL_SCANCODE_S:
        {
            keyboard_scancode = 0x1F;
            break;
        }
        case SDL_SCANCODE_T:
        {
            keyboard_scancode = 0x14;
            break;
        }
        case SDL_SCANCODE_U:
        {
            keyboard_scancode = 0x16;
            break;
        }
        case SDL_SCANCODE_V:
        {
            keyboard_scancode = 0x2F;
            break;
        }
        case SDL_SCANCODE_W:
        {
            keyboard_scancode = 0x11;
            break;
        }
        case SDL_SCANCODE_X:
        {
            keyboard_scancode = 0x2D;
            break;
        }
        case SDL_SCANCODE_Y:
        {
            keyboard_scancode = 0x15;
            break;
        }
        case SDL_SCANCODE_Z:
        {
            keyboard_scancode = 0x2C;
            break;
        }
        case SDL_SCANCODE_0:
        {
            keyboard_scancode = 0x0B;
            break;
        }
        case SDL_SCANCODE_1:
        {
            keyboard_scancode = 0x02;
            break;
        }
        case SDL_SCANCODE_2:
        {
            keyboard_scancode = 0x03;
            break;
        }
        case SDL_SCANCODE_3:
        {
            keyboard_scancode = 0x04;
            break;
        }
        case SDL_SCANCODE_4:
        {
            keyboard_scancode = 0x05;
            break;
        }
        case SDL_SCANCODE_5:
        {
            keyboard_scancode = 0x06;
            break;
        }
        case SDL_SCANCODE_6:
        {
            keyboard_scancode = 0x07;
            break;
        }
        case SDL_SCANCODE_7:
        {
            keyboard_scancode = 0x08;
            break;
        }
        case SDL_SCANCODE_8:
        {
            keyboard_scancode = 0x09;
            break;
        }
        case SDL_SCANCODE_9:
        {
            keyboard_scancode = 0x0A;
            break;
        }
        case SDL_SCANCODE_GRAVE:
        {
            keyboard_scancode = 0x29;
            break;
        }
        case SDL_SCANCODE_MINUS:
        {
            keyboard_scancode = 0x0C;
            break;
        }
        case SDL_SCANCODE_EQUALS:
        {
            keyboard_scancode = 0x0E;
            break;
        }
        case SDL_SCANCODE_BACKSLASH:
        {
            keyboard_scancode = 0x2B;
            break;
        }
        case SDL_SCANCODE_BACKSPACE:
        {
            keyboard_scancode = 0x0E;
            break;
        }
        case SDL_SCANCODE_SPACE:
        {
            keyboard_scancode = 0x39;
            break;
        }
        case SDL_SCANCODE_TAB:
        {
            keyboard_scancode = 0x0F;
            break;
        }
        case SDL_SCANCODE_CAPSLOCK:
        {
            keyboard_scancode = 0x3A;
            break;
        }
        case SDL_SCANCODE_LSHIFT:
        {
            keyboard_scancode = 0x2A;
            break;
        }
        case SDL_SCANCODE_LGUI:
        {
            keyboard_scancode = 0xE0;
            break;
        }
        }
        
        if(event.type == SDL_EVENT_KEY_UP)
        {
            keyboard_scancode += 80;
        }

        sys->ppi.regs[0] = keyboard_scancode;
        sys->pic_master.irr |= 1 << PIC_IRQ_KEYBOARD;
    }
}
