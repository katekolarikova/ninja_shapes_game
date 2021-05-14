#pragma once

#include <chipmunk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include "sdl.h"
#include "dynamic_array.h"
#include <time.h>

typedef struct
{
    cpBody *body;
    cpShape *shape;
    int r;
    int g;
    int b;
    int life;
} Fruit;

// vytvoreni noveho shapu a jeho pridani do prostoru
void create_fruit(cpSpace *space, dynarray *blocks, SDL_Renderer *renderer);