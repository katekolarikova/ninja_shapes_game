#pragma once
#include <chipmunk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include "sdl.h"
#include "dynamic_array.h"
#include <time.h>
#include "rezani.h"
#include "ovoce.h"

typedef struct
{
    int x_start;
    int x_end;
    int y_start;
    int y_end;
    int alpha;
    int process;
} Line;

//vytiskne pohyb za mecem
void print_line(dynarray *lines, int x_mouse, int y_mouse, SDL_Renderer *renderer);

//vytiskne jednotlive tvary
void print_shapes(dynarray *blocks, SDL_Renderer *renderer, int *lives, int window_height);

//vytiskne text - body, levly, zivoty
void print_text(int lives, int points, int level, SDL_Renderer *renderer, TTF_Font *font);

//tisk mece
void print_sword(int x_sword, int y_sword,SDL_Renderer *renderer, SDL_Texture *image, int sword_degree );

//zmena pruhlednosti
void change_line_alpha(dynarray* lines);