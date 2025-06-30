#include <termmanip.h>
#include "system.h"

int main(int argc, char** argv) {
    if (argc < 2)
    {
        printf("Usage: %s [IMAGE]\n", argv[0]);
        return 1;
    }

    FILE* image = fopen(argv[1], "r+b");

    if (image == NULL)
    {
        fprintf(stderr, "Invalid image %s.", argv[1]);
        return 2;
    }

    Tm_terminal* terminal = tm_terminal();

    if (terminal == NULL)
    {
        fprintf(stderr, "Couldn't initalize termmanip\n");
        return 3;
    }

    int term_cols, term_rows;

    tm_get_termsize(&term_cols, &term_rows);

    Sys8086* sys = init_sys(image);

    int emuwin_cols = sys->display.columns;
    int emuwin_rows = sys->display.rows;

    Tm_window* emuwin = tm_window(terminal, "emuwin", 1, 1, emuwin_cols, emuwin_rows, NULL, TM_WIN_NONE, 0);
    Tm_window* debugwin = tm_window(terminal, "debugwin", emuwin_cols + 3, 1, term_cols - emuwin_cols - 4, 25, NULL, TM_WIN_NONE, 0);

    if (emuwin == NULL || debugwin == NULL)
    {
        fprintf(stderr, "Couldn't initalize termmanip\n");
        return 4;
    }

    tm_win_flags(emuwin, TM_FLAG_BORDER, 1);
    tm_win_flags(emuwin, TM_FLAG_TERMINAL_INPUT, 1);
    tm_win_flags(emuwin, TM_FLAG_ECHO, 0);
    tm_win_flags(emuwin, TM_FLAG_CURSOR_VISIBLE, 0);

    tm_win_input_timeout(emuwin, 0);

    tm_win_flags(debugwin, TM_FLAG_BORDER, 1);
    tm_win_flags(debugwin, TM_FLAG_SCROLL, 1);
    tm_win_flags(debugwin, TM_FLAG_WRAP_TEXT, 1);

    Tm_input input = { 0 };

    while (input.key != 'q')
    {
        cpu_exec(sys);
        tm_win_clear(debugwin);
        tm_win_clear(emuwin);

        for (int y = 0; y < emuwin_rows; y++)
        {
            for (int x = 0; x < emuwin_cols; x++)
            {
                tm_win_cursor(emuwin, x, y);

                tm_win_attrib(emuwin, TM_ATTRIB_ALL, 0);

                uint8_t bg_color = (sys->display.mem[((y * sys->display.columns + x)) * (sys->display.display_page + 1)].attrib & 0b11110000) >> 4;
                uint8_t fg_color = sys->display.mem[((y * sys->display.columns + x)) * (sys->display.display_page + 1)].attrib & 0b00001111;
                
                switch(fg_color)
                {
                case 0x0:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BLACK, 1);
                    break;
                case 0x1:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BLUE, 1);
                    break;
                case 0x2:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_GREEN, 1);
                    break;
                case 0x3:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_CYAN, 1);
                    break;
                case 0x4:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_RED, 1);
                    break;
                case 0x5:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_MAGENTA, 1);
                    break;
                case 0x6:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_YELLOW, 1);
                    break;
                case 0x7:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_WHITE, 1);
                    break;
                case 0x8:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTBLACK, 1);
                    break;
                case 0x9:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTBLUE, 1);
                    break;
                case 0xA:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTGREEN, 1);
                    break;
                case 0xB:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTCYAN, 1);
                    break;
                case 0xC:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTRED, 1);
                    break;
                case 0xD:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTMAGENTA, 1);
                    break;
                case 0xE:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTYELLOW, 1);
                    break;
                case 0xF:
                    tm_win_attrib(emuwin, TM_ATTRIB_FG_BRIGHTWHITE, 1);
                    break;
                }

                switch (bg_color)
                {
                case 0x0:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BLACK, 1);
                    break;
                case 0x1:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BLUE, 1);
                    break;
                case 0x2:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_GREEN, 1);
                    break;
                case 0x3:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_CYAN, 1);
                    break;
                case 0x4:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_RED, 1);
                    break;
                case 0x5:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_MAGENTA, 1);
                    break;
                case 0x6:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_YELLOW, 1);
                    break;
                case 0x7:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_WHITE, 1);
                    break;
                case 0x8:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTBLACK, 1);
                    break;
                case 0x9:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTBLUE, 1);
                    break;
                case 0xA:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTGREEN, 1);
                    break;
                case 0xB:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTCYAN, 1);
                    break;
                case 0xC:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTRED, 1);
                    break;
                case 0xD:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTMAGENTA, 1);
                    break;
                case 0xE:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTYELLOW, 1);
                    break;
                case 0xF:
                    tm_win_attrib(emuwin, TM_ATTRIB_BG_BRIGHTWHITE, 1);
                    break;
                }

                tm_win_print(emuwin, "%c", sys->display.mem[((y * sys->display.columns + x)) * (sys->display.display_page + 1)].disp);
            }
        }

        tm_win_print(debugwin, "AX: %x, BX: %x, CX: %x, DX: %x, CS: %x, DS: %x, SS: %x, ES: %x, SP: %x, BP: %x, SI: %x, DI: %x, IP: %x, FLAGS: %x",
            sys->cpu.ax.whole, sys->cpu.bx.whole, sys->cpu.cx.whole, sys->cpu.dx.whole, sys->cpu.cs.whole, sys->cpu.ds.whole, sys->cpu.ss.whole, sys->cpu.es.whole, sys->cpu.sp.whole, sys->cpu.bp.whole, sys->cpu.si.whole, sys->cpu.di.whole, sys->cpu.ip.whole, sys->cpu.flag.whole);
        tm_win_print(debugwin, "\nLast 10 items on stack: \n");

        for (int i = 0; i < 10; i++)
        {
            tm_win_print(debugwin, "%x\n", sys->memory[sys->cpu.ss.whole * 0x10 + sys->cpu.sp.whole + i]);
        }
        
        tm_win_cursor(debugwin, 0, 0);
        tm_win_mark_for_update(debugwin);
        input = tm_win_input(emuwin);

        if (input.terminal_resized)
        {
            tm_get_termsize(&term_cols, &term_rows);
            tm_win_modify(debugwin, emuwin_cols + 3, 1, term_cols - emuwin_cols - 4, 25, 0);
        }
    }

    tm_terminal_free(terminal);
}