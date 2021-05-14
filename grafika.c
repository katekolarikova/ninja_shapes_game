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
#include "grafika.h"

void print_line(dynarray *lines, int x_mouse, int y_mouse, SDL_Renderer *renderer)
{

    for (int i = 0; i < lines->size; i++)
    {
        Line *l = lines->items[i];
        SDL_SetRenderDrawColor(renderer, 192, 192, 192, l->alpha);

        // pokud je stisknuta mys v pohybu, koncim v poloze mysi, jinak tam, kde byla pustena
        if (l->process == 1)
        {
            l->x_end = x_mouse;
            l->y_end = y_mouse;
            SDL_RenderDrawLine(renderer, l->x_start, l->y_start, l->x_end, l->y_end);
        }
        else
        {
            SDL_RenderDrawLine(renderer, l->x_start, l->y_start, l->x_end, l->y_end);
        }
    }
}

void print_shapes(dynarray *blocks, SDL_Renderer *renderer, int *lives, int window_height)
{
    for (int i= 0; i < blocks->size; i++)
    {
        Fruit *f = blocks->items[i];
        int countTops = cpPolyShapeGetCount(f->shape);
        SDL_Point *points = (SDL_Point *)malloc(sizeof(SDL_Point) * (countTops + 1));
        cpVect position = cpBodyGetPosition(f->body);
        if (position.y < 0)
        {
            dynarray_remove(blocks, f); 
            if (f->life == 1)
            {
                *lives -= 1;
            }
            i-=1;
        }
        else
        {
            float angle_radians = cpBodyGetAngle(f->body);  // Získání úhlu kostky (v radiánech)
            float angle_deg = angle_radians * (180 / M_PI); // Převod na stupně
            for (int j = 0; j < countTops; j++)
            {
                cpVect a = cpBodyLocalToWorld(f->body, cpPolyShapeGetVert(f->shape, j));
                int x = a.x;
                int y = window_height - a.y;
                SDL_Point p = {x, y};
                points[j] = p;
            }
            SDL_Point p = points[0];
            points[countTops] = p;

            SDL_SetRenderDrawColor(renderer, f->r, f->g, f->b, 255);
            SDL_RenderDrawLines(renderer, points, countTops + 1);
        }
        free(points);
    }
}

void print_text(int lives, int points, int level, SDL_Renderer *renderer, TTF_Font *font)
{
    //zivoty
    char text_live[50];
    snprintf(text_live, sizeof(text_live), "Lives: %d", lives);
    sdl_draw_text(renderer, font, (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255},
                  (SDL_Rect){
                      .x = 670,
                      .y = 0,
                      .w = 110,
                      .h = 50},
                  text_live);

    //body
    char text_points[50];
    snprintf(text_points, sizeof(text_points), "Points: %d", points);
    sdl_draw_text(renderer, font, (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255},
                  (SDL_Rect){
                      .x = 10,
                      .y = 51,
                      .w = 100,
                      .h = 50},
                  text_points);

    //levly
    char text_level[50];
    snprintf(text_level, sizeof(text_level), "Level: %d", level);
    sdl_draw_text(renderer, font, (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255},
                  (SDL_Rect){
                      .x = 10,
                      .y = 0,
                      .w = 100,
                      .h = 50},
                  text_level);
}

void print_sword(int x_sword, int y_sword,SDL_Renderer *renderer, SDL_Texture *image, int sword_degree )
{
    SDL_Rect rect = {
        .x = x_sword - 20-25, 
        .y = y_sword -20 ,//-20
        .w = 100,
        .h = 30};

    SDL_RenderCopyEx(renderer, image, NULL, &rect, -sword_degree, NULL, SDL_FLIP_NONE);
}

void change_line_alpha(dynarray* lines)
{

    for (int i = 0; i < lines->size; i++)
    {
        Line *l = lines->items[i];
        if (l->alpha - 20 < 0)
        {
            dynarray_remove(lines, l);
        }
        else
        {
            l->alpha -= 20;
        }
    }
}