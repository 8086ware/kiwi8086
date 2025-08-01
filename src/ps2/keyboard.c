#include "system.h"
#include <SDL3/SDL_events.h>
#include "memory.h"

void ps2_keyboard(Sys8086* sys, SDL_Event event)
{
    uint8_t ps2_keyboard_scancode = 0;
    // scancode set 1
    
    if(event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN)
    {
        static _Bool two_byte = 0;
        static SDL_Scancode scan_code = 0;
    
        switch(event.key.scancode)
        {
        case SDL_SCANCODE_A:
        {
            ps2_keyboard_scancode = 0x1E;
            break;
        }
        case SDL_SCANCODE_B:
        {
            ps2_keyboard_scancode = 0x30;
            break;
        }
        case SDL_SCANCODE_C:
        {
            ps2_keyboard_scancode = 0x2E;
            break;
        }
        case SDL_SCANCODE_D:
        {
            ps2_keyboard_scancode = 0x20;
            break;
        }
        case SDL_SCANCODE_E:
        {
            ps2_keyboard_scancode = 0x12;
            break;
        }
        case SDL_SCANCODE_F:
        {
            ps2_keyboard_scancode = 0x21;
            break;
        }
        case SDL_SCANCODE_G:
        {
            ps2_keyboard_scancode = 0x22;
            break;
        }
        case SDL_SCANCODE_H:
        {
            ps2_keyboard_scancode = 0x23;
            break;
        }
        case SDL_SCANCODE_I:
        {
            ps2_keyboard_scancode = 0x17;
            break;
        }
        case SDL_SCANCODE_J:
        {
            ps2_keyboard_scancode = 0x24;
            break;
        }
        case SDL_SCANCODE_K:
        {
            ps2_keyboard_scancode = 0x25;
            break;
        }
        case SDL_SCANCODE_L:
        {
            ps2_keyboard_scancode = 0x26;
            break;
        }
        case SDL_SCANCODE_M:
        {
            ps2_keyboard_scancode = 0x32;
            break;
        }
        case SDL_SCANCODE_N:
        {
            ps2_keyboard_scancode = 0x31;
            break;
        }
        case SDL_SCANCODE_O:
        {
            ps2_keyboard_scancode = 0x18;
            break;
        }
        case SDL_SCANCODE_P:
        {
            ps2_keyboard_scancode = 0x19;
            break;
        }
        case SDL_SCANCODE_Q:
        {
            ps2_keyboard_scancode = 0x10;
            break;
        }
        case SDL_SCANCODE_R:
        {
            ps2_keyboard_scancode = 0x13;
            break;
        }
        case SDL_SCANCODE_S:
        {
            ps2_keyboard_scancode = 0x1F;
            break;
        }
        case SDL_SCANCODE_T:
        {
            ps2_keyboard_scancode = 0x14;
            break;
        }
        case SDL_SCANCODE_U:
        {
            ps2_keyboard_scancode = 0x16;
            break;
        }
        case SDL_SCANCODE_V:
        {
            ps2_keyboard_scancode = 0x2F;
        break;
        }
        case SDL_SCANCODE_W:
        {
            ps2_keyboard_scancode = 0x11;
            break;
        }
        case SDL_SCANCODE_X:
        {
            ps2_keyboard_scancode = 0x2D;
            break;
        }
        case SDL_SCANCODE_Y:
        {
            ps2_keyboard_scancode = 0x15;
            break;
        }
        case SDL_SCANCODE_Z:
        {
            ps2_keyboard_scancode = 0x2C;
            break;
        }
        case SDL_SCANCODE_0:
        {
            ps2_keyboard_scancode = 0x0B;
            break;
        }
        case SDL_SCANCODE_1:
        {
            ps2_keyboard_scancode = 0x02;
            break;
        }
        case SDL_SCANCODE_2:
        {
            ps2_keyboard_scancode = 0x03;
            break;
        }
        case SDL_SCANCODE_3:
        {
            ps2_keyboard_scancode = 0x04;
            break;
        }
        case SDL_SCANCODE_4:
        {
            ps2_keyboard_scancode = 0x05;
            break;
        }
        case SDL_SCANCODE_5:
        {
            ps2_keyboard_scancode = 0x06;
            break;
        }
        case SDL_SCANCODE_6:
        {
            ps2_keyboard_scancode = 0x07;
            break;
        }
        case SDL_SCANCODE_7:
        {
            ps2_keyboard_scancode = 0x08;
            break;
        }
        case SDL_SCANCODE_8:
        {
            ps2_keyboard_scancode = 0x09;
            break;
        }
        case SDL_SCANCODE_9:
        {
            ps2_keyboard_scancode = 0x0A;
            break;
        }
        case SDL_SCANCODE_GRAVE:
        {
            ps2_keyboard_scancode = 0x29;
            break;
        }
        case SDL_SCANCODE_MINUS:
        {
            ps2_keyboard_scancode = 0x0C;
            break;
        }
        case SDL_SCANCODE_EQUALS:
        {
            ps2_keyboard_scancode = 0x0E;
            break;
        }
        case SDL_SCANCODE_BACKSLASH:
        {
            ps2_keyboard_scancode = 0x2B;
            break;
        }
        case SDL_SCANCODE_BACKSPACE:
        {
            ps2_keyboard_scancode = 0x0E;
            break;
        }
        case SDL_SCANCODE_SPACE:
        {
            ps2_keyboard_scancode = 0x39;
            break;
        }
        case SDL_SCANCODE_TAB:
        {
            ps2_keyboard_scancode = 0x0F;
            break;
        }
        case SDL_SCANCODE_CAPSLOCK:
        {
            ps2_keyboard_scancode = 0x3A;
            break;
        }
        case SDL_SCANCODE_LSHIFT:
        {
            ps2_keyboard_scancode = 0x2A;
            break;
        }
        case SDL_SCANCODE_LGUI:
        {
            ps2_keyboard_scancode = 0xE0;
            break;
        }
        }
        
        if(event.type == SDL_EVENT_KEY_UP)
        {
            ps2_keyboard_scancode += 80;
        }

        sys->ps2.output_buffer = ps2_keyboard_scancode;
        sys->ps2.status_reg |= PS2_OUTPUT_BUFFER; // Output is available
        sys->pic_master.irr |= 1 << PIC_IRQ_KEYBOARD;
    }
}
