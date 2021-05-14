#pragma once

#include <chipmunk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include "sdl.h"
#include "dynamic_array.h"
#include <time.h>

#define DENSITY (1.0 / 10000.0)
#define GRABBABLE_MASK_BIT (1 << 31)

typedef struct 
{
    cpVect a, b;
    cpSpace *space;
    dynarray *blocks;
    int * points;
    int * level;
    float * new_fruit;
}SliceContext;

void ClipPoly(cpSpace *space, cpShape *shape, cpVect n, cpFloat dist, dynarray *blocks, int smer);
void SliceShapePostStep(cpSpace *space, cpShape *shape, SliceContext *context);
void SliceQuery(cpShape *shape, cpVect point, cpVect normal, cpFloat alpha, SliceContext *context);