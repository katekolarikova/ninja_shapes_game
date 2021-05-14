#include <chipmunk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include "sdl.h"
#include "dynamic_array.h"
#include <time.h>
#include "ovoce.h"
#include "rezani.h"

void create_fruit(cpSpace *space, dynarray *blocks, SDL_Renderer *renderer)
{
    Fruit *f = (Fruit *)malloc(sizeof(Fruit));

    int v1 = (rand() % 61) - 30;
    int angl = (rand() % 200) - 100;
    int v2 = rand() % (250 - 200 + 1) + 200;
    float x = rand() % (650 - 150 + 1) + 150;
    float pocet_bodu = rand() % (5 - 3 + 1) + 3;
    int rozmer = rand() % (90 - 50 + 1) + 50;
    int rozmer2 = rand() % (90 - 50 + 1) + 50;
    int r = (rand() % 255);
    int g = (rand() % 255);
    int b = (rand() % 255);
    f->r = r;
    f->g = g;
    f->b = b;
    cpVect *souradnice = (cpVect *)malloc((pocet_bodu) * sizeof(cpVect));

    if (pocet_bodu == 3)
    {
        cpVect prvni = {.x = x, .y = 0};
        souradnice[0] = prvni;
        cpVect druha = {.x = x + rozmer, .y = 0};
        souradnice[1] = druha;
        cpVect treti = {.x = x + (rozmer / 2), .y = rozmer};
        souradnice[2] = treti;
    }

    if (pocet_bodu == 4)
    {
        cpVect prvni = {.x = x, .y = 0};
        souradnice[0] = prvni;
        cpVect druha = {.x = x + rozmer, .y = 0};
        souradnice[1] = druha;
        cpVect treti = {.x = x + rozmer, .y = rozmer2};
        souradnice[2] = treti;
        cpVect ctvrta = {.x = x, .y = rozmer2};
        souradnice[3] = ctvrta;
    }

    if (pocet_bodu == 5)
    {
        cpVect prvni = {.x = x, .y = 0};
        souradnice[0] = prvni;
        cpVect druha = {.x = x + rozmer, .y = 0};
        souradnice[1] = druha;
        cpVect treti = {.x = x + rozmer + rozmer / 2, .y = rozmer};
        souradnice[2] = treti;
        cpVect ctvrta = {.x = x + rozmer / 2, .y = 2 * rozmer};
        souradnice[3] = ctvrta;
        cpVect pata = {.x = x - rozmer / 2, .y = rozmer};
        souradnice[4] = pata;
    }

    cpVect centroid = cpCentroidForPoly(pocet_bodu, souradnice);                            //teziste
    cpFloat mass = cpAreaForPoly(pocet_bodu, souradnice, 0.0f) * DENSITY;                   // hmotnost
    cpFloat moment = cpMomentForPoly(mass, pocet_bodu, souradnice, cpvneg(centroid), 0.0f); // moment hybnosti

    f->body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
    cpBodySetPosition(f->body, centroid);
    cpBodySetVelocity(f->body, cpv(v1, v2));
    cpBodySetAngularVelocity(f->body, 1.0f);

    cpTransform transform = cpTransformTranslate(cpvneg(centroid));
    f->shape = cpPolyShapeNew(f->body, pocet_bodu, souradnice, transform, 0.0);
    cpSpaceAddShape(space, f->shape);

    cpShapeSetFriction(f->shape, cpShapeGetFriction(f->shape));
    cpShapeSetSensor(f->shape, 1);
    f->life = 1;

    dynarray_push(blocks, f);
}