#include "display/cga.h"
#include "display/crtc.h"
#include "display/font.h"
#include "system.h"
#include <SDL3/SDL_render.h>
#include "memory.h"
#include <stdlib.h>

void display_render(Sys8086* sys)
{
    SDL_ClearSurface(sys->display.surface, 0, 0, 0, 0);
    
    SDL_Texture* screen_texture = NULL;

    int width = 0;
    int height = 0;
    int columns = 0;
    int rows = 0;

    int bpp = 0;

    if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_VIDEO) // Is this thing on?
	{
        if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_GRAPHICS)
        {
            // graphics mode

            if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_HIGH_RES_GRAPHICS) // 640x200 2 colors (1bpp, 640x200/8 = 16k)
            {
                width = 640;
                height = 200;
                bpp = 1;
            }

            else // 320x200 4 colors (2 bits per color, 320x200/(8/2) = 16k)
            {
                width = 320;
                height = 200;
                bpp = 2;
            }

            memcpy(sys->display.surface->pixels, sys->display.cga.ram, (width*height)/(8/bpp));
        }

        else
        {
            // text mode

            // 80x25 (high res text, 640x200, 16 colors)

            if(sys->display.cga.mode_ctrl_reg & CGA_MODE_CONTROL_HIGH_RES)
            {
                width = 640;
                height = 200;
                columns = 80;
                rows = 25;

                bpp = 4;
            }

            else //320x200 4bpp text mode(40x25)
            {
                width = 320;
                height = 200;
                columns = 40;
                rows = 25;
                
                bpp = 4;
            }

            for(int y = 0; y < rows; y++)
            {
                for(int x = 0; x < columns; x++)
                {
                    uint8_t fg_color = sys->display.cga.ram[(sys->display.crtc.start_address * 2) + ((y * columns + x) * 2 + 1)] & 0x0f;
                    uint8_t bg_color = (sys->display.cga.ram[(sys->display.crtc.start_address * 2) + ((y * columns + x) * 2 + 1)] & 0xf0) >> 4;
                    uint8_t character = sys->display.cga.ram[(sys->display.crtc.start_address * 2) + ((y * columns + x) * 2)];

                    int write_location_y = 8 * y;
                    int write_location_x = 8 * x;

                    int font_location = 8 * character;
                            
                    for(int font_y = 0; font_y < 8; font_y++)
                    {
                        for(int font_x = 0; font_x < 8; font_x += 2)
                        {
                            if((font[font_location + font_y] << font_x) & 0b10000000)
                            {
                                ((uint8_t*)sys->display.surface->pixels)[(write_location_y + font_y) * (width/(8/bpp)) + (write_location_x / (8/bpp) + font_x / (8/bpp))] |= (fg_color << 4);
                            }

                            else
                            {
                                ((uint8_t*)sys->display.surface->pixels)[(write_location_y + font_y) * (width/(8/bpp)) + (write_location_x / (8/bpp) + font_x / (8/bpp))] |= (bg_color << 4);
                            }

                            if((font[font_location + font_y] << font_x) & 0b01000000)
                            {
                                ((uint8_t*)sys->display.surface->pixels)[(write_location_y + font_y) * (width/(8/bpp)) + (write_location_x / (8/bpp) + font_x / (8/bpp))] |= fg_color;
                            }

                            else
                            {
                                ((uint8_t*)sys->display.surface->pixels)[(write_location_y + font_y) * (width/(8/bpp)) + (write_location_x / (8/bpp) + font_x / (8/bpp))] |= bg_color;
                            }
                        }
                    }
                }
            }

            int cursor_y = sys->display.crtc.cursor_address / 80;
            int cursor_x = sys->display.crtc.cursor_address % 80;

            int write_location_y = cursor_y * 8;
            int write_location_x = cursor_x * 4;

            for(int y = write_location_y + sys->display.crtc.cursor_start_scan_line; y < write_location_y + sys->display.crtc.cursor_end_scan_line; y++)
            {
                for(int x = write_location_x; x < write_location_x + 8/(8/bpp); x++)
                {
                    ((uint8_t*)sys->display.surface->pixels)[y * (width/(8/bpp)) + x] = 0xFF;
                }
            }
        }
    }

    screen_texture = SDL_CreateTextureFromSurface(sys->display.win_render, sys->display.surface);
    SDL_SetTextureScaleMode(screen_texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(sys->display.win_render, screen_texture, NULL, NULL);
    SDL_RenderPresent(sys->display.win_render);
    SDL_DestroyTexture(screen_texture);
}
