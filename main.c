/*
Katerina Kolarikova
1.1.21
zaverecny projekt - hra Ninja Shapes
ukolem je presekavani tvaru - nesmi nepreseknute prepadnout pod dolni okraj. pokud se tak stane prijde hrac o zivot
soubory:
    main.c
    rezani.h, rezani.c
    ovoce.c, ovoce.h
    grafika.c, grafika.h
    dynamic_array.h, dynamic_array.c
    sdl.c, sdl.h
import:
    Arial.ttf
    savle.svg
*/

#include <chipmunk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include "sdl.h"
#include "dynamic_array.h"
#include "rezani.h"
#include "ovoce.h"
#include "grafika.h"

cpShapeFilter GRAB_FILTER = {CP_NO_GROUP, GRABBABLE_MASK_BIT, GRABBABLE_MASK_BIT};

int main()
{
    srand(time(NULL));

    // parametry okna
    int window_width = 800;
    int window_height = 600;

    //casovy krok fyziky
    float timestep = 1.0f / 60.0f;

    // vytvoreni rendereru
    SDL_Context ctx = sdl_context_init("Ninja Shapes", window_width, window_height);
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    bool quit = false;
    bool game_over_quit = true;
    SDL_ShowCursor(SDL_DISABLE);

    //nacteni fontu
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("Arial.ttf", 100);
    assert(font);

    //nactni obrazku mece
    SDL_Texture *image = IMG_LoadTexture(ctx.renderer, "savle.svg");
    if (!image)
    {
        fprintf(stderr, "Obrazek nenalezen\n");
        return 1;
    }

    // pole shapu
    dynarray blocks;
    dynarray_init(&blocks, 20);

    // pole seku
    dynarray lines;
    dynarray_init(&lines, 20);

    //vytvoreni fzyikalniho prostoru
    cpSpace *space = cpSpaceNew();
    cpSpaceSetGravity(space, cpv(0, -70));
    cpSpaceSetSleepTimeThreshold(space, 0.5f);
    cpSpaceSetCollisionSlop(space, 0.5f);

    //pocitadla
    Uint64 last = SDL_GetPerformanceCounter();
    float fruit_counter = 0.0f;
    float new_fruit = 5.0f;
    float low_alpha = 0.1f;
    float alpha_counter = 0.0f;

    cpVect start = {};
    cpVect end = {};
    int x_sword;
    int y_sword;
    int sword_degree = 90;

    int lives = 3;
    int points = 0;
    int level = 1;

    // vytvoreni prvniho ovoce
    create_fruit(space, &blocks, ctx.renderer);

    // hlavni herni smycka
    while (!quit)
    {
        Uint64 now = SDL_GetPerformanceCounter();

        // Počet vteřin od poslední iterace herní smyčky
        float delta_time_s = ((float)(now - last) / (float)SDL_GetPerformanceFrequency());
        last = now;

        fruit_counter += delta_time_s;
        alpha_counter += delta_time_s;

        while (fruit_counter >= new_fruit)
        {

            create_fruit(space, &blocks, ctx.renderer);
            fruit_counter -= new_fruit;
        }

        while (alpha_counter >= low_alpha)
        {
            change_line_alpha(&lines);
            alpha_counter -= low_alpha;
        }

        // prubeh fyziky
        cpSpaceStep(space, timestep);

        int x_mouse;
        int y_mouse;
        SDL_GetMouseState(&x_mouse, &y_mouse);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) //quit - stiknuti krizku
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                x_sword = event.motion.x;
                y_sword = event.motion.y;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) // stisknuti mysi
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    start.x = event.button.x;
                    start.y = window_height - event.button.y;
                    sword_degree = 45;

                    Line *l = (Line *)malloc(sizeof(Line));
                    l->x_start = event.button.x;
                    l->y_start = event.button.y;
                    l->alpha = 255;
                    l->process = 1;
                    dynarray_push(&lines, l);
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) // pusteni mysi
            {

                end.x = event.button.x;
                end.y = window_height - event.button.y;
                sword_degree = 90;

                for (int i = 0; i < lines.size; i++)
                {
                    Line *l = lines.items[i];
                    l->process = 0;
                }
                // okolnosi, ktere chci pri callbacku predat
                SliceContext context = {.a = start, .b = end, .space = space, .blocks = &blocks, .points = &points, .level = &level, .new_fruit = &new_fruit};

                // zacatek potencialniho rezaciho procesu
                cpSpaceSegmentQuery(space, start, end, 0.0, GRAB_FILTER, (cpSpaceSegmentQueryFunc)SliceQuery, &context); // projde body a pokud protnuly tvar, zavola SliceQuery
            }
        }

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 225); //barva pozadi
        SDL_RenderClear(ctx.renderer);

        //tisk stavu hry
        print_line(&lines, x_mouse, y_mouse, ctx.renderer);
        print_shapes(&blocks, ctx.renderer, &lives, window_height);
        print_text(lives, points, level, ctx.renderer, font);
        print_sword(x_sword, y_sword, ctx.renderer, image, sword_degree);

        SDL_RenderPresent(ctx.renderer);

        //kontrola poctu zivotu
        if (lives == 0)
        {
            quit = true;
            game_over_quit = false;
        }
    }

    //vytisteni konce hry
    SDL_ShowCursor(SDL_ENABLE);
    while (!game_over_quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) //quit - stiknuti krizku
            {
                game_over_quit = true;
            }
        }

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 225); //barva pozadi
        SDL_RenderClear(ctx.renderer);

        char text_game[50];
        snprintf(text_game, sizeof(text_game), "Game over, you have %d points", points);
        sdl_draw_text(ctx.renderer, font, (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255},
                      (SDL_Rect){
                          .x = 150,
                          .y = 100,
                          .w = 500,
                          .h = 100},
                      text_game);
        SDL_RenderPresent(ctx.renderer);
    }

    // uvolneni pameti

    for (int i = 0; i < blocks.size; i++)
    {
        Fruit *f = blocks.items[i];
        cpSpaceRemoveShape(space, f->shape);
        cpSpaceRemoveBody(space, f->body);
        cpShapeFree(f->shape);
        cpBodyFree(f->body);
    }
    cpSpaceFree(space);
    dynarray_free(&blocks);
    dynarray_free(&lines);
    SDL_DestroyTexture(image);
    TTF_CloseFont(font);
    TTF_Quit;
    sdl_context_free(&ctx);
    SDL_Quit();
    return 0;
}
