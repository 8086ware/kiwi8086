#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL3/SDL_events.h>
#include "system.h"

void poll_keyboard(Sys8086* sys, SDL_Event event);

#endif