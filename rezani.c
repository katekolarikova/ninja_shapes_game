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

cpShapeFilter NOT_GRABBABLE_FILTER = {CP_NO_GROUP, ~GRABBABLE_MASK_BIT, ~GRABBABLE_MASK_BIT};

void ClipPoly(cpSpace *space, cpShape *shape, cpVect n, cpFloat dist, dynarray *blocks, int smer)
{
    cpBody *body = cpShapeGetBody(shape);
    int count = cpPolyShapeGetCount(shape); // pocet vrcholu polygonu
    int clippedCount = 0;

    cpVect *clipped = (cpVect *)malloc((count + 1) * sizeof(cpVect)); //pole, kam muzu ukladat nove vrcholy

    for (int i = 0, j = count - 1; i < count; j = i, i++)
    {
        cpVect a = cpBodyLocalToWorld(body, cpPolyShapeGetVert(shape, j)); // prevede relativni souradnice ( pociaji se od teziste telesa), na klasicke kartezske
        cpFloat a_dist = cpvdot(a, n) - dist;                              // pro tenhle shape mi vrat souradnice j-teho vrcholu ( predchoziho)

        // podle toho, jestli je 'a' zaporne nebo kladne urci, na ktere strane rezu lezi - pokud je negativni, lezi na spravne strane
        if (a_dist < 0.0)
        {
            clipped[clippedCount] = a; // pridam souradnice 'a' k souradnicim nove vynikajiciho
            clippedCount++;            // zvysim index, na ktery budu priste zapisovat
        }

        cpVect b = cpBodyLocalToWorld(body, cpPolyShapeGetVert(shape, i)); // vrati souradnice i-teho (soucasneho) vrcholu
        cpFloat b_dist = cpvdot(b, n) - dist;

        // kontrolaz jestli je soucin souradnic predchoziho a nasledujiciho bodu kladne nebo zaporne
        // pokud jsou zaaporne, znamena to, ze kaza lezi v jinem z nove vynikajicich utvaru -> mam novy bod, ktery lezi mezi nimi
        if (a_dist * b_dist < 0.0f)
        {
            cpFloat t = cpfabs(a_dist) / (cpfabs(a_dist) + cpfabs(b_dist)); // t - jak daleko jsme od 'a' dp 'b'  v <0;1>

            clipped[clippedCount] = cpvlerp(a, b, t);
            clippedCount++;
        }
    }

    // pridani puvodnich vlastnosti k novemu objektu
    
    Fruit *f = (Fruit *)malloc(sizeof(Fruit));
    cpVect centroid = cpCentroidForPoly(clippedCount, clipped);                            //teziste
    cpFloat mass = cpAreaForPoly(clippedCount, clipped, 0.0f) * DENSITY;                   // hmotnost
    cpFloat moment = cpMomentForPoly(mass, clippedCount, clipped, cpvneg(centroid), 0.0f); // moment hybnosti

    f->body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
    cpBodySetPosition(f->body, centroid);
    cpVect velocity= cpBodyGetVelocityAtWorldPoint(body, centroid);
    velocity.x=velocity.x-(100*smer);
    velocity.y=velocity.y-300;
    cpBodySetVelocity(f->body,velocity);
    cpFloat angular=cpBodyGetAngularVelocity(body);
    angular=angular*3*smer;
    cpBodySetAngularVelocity(f->body,angular);

    cpTransform transform = cpTransformTranslate(cpvneg(centroid));
    f->shape = cpPolyShapeNew(f->body, clippedCount, clipped, transform, 0.0);
    cpSpaceAddShape(space, f->shape);

    cpShapeSetFriction(f->shape, cpShapeGetFriction(shape));
    cpShapeSetSensor(f->shape, 1);
    cpShapeSetFilter(f->shape, NOT_GRABBABLE_FILTER);

    for (int i = 0; i < blocks->size; i++)
    {
        Fruit *f_puv = (Fruit *)blocks->items[i];
        if (f_puv->shape == shape)
        {
            f->r = f_puv->r;
            f->g = f_puv->g;
            f->b = f_puv->b;
            break;
        }
    }

    f->life = 0;

    dynarray_push(blocks, f);
    free(clipped);
    
}

void SliceShapePostStep(cpSpace *space, cpShape *shape, SliceContext *context)
{
    cpVect a = context->a;
    cpVect b = context->b;
    // Clipping plane normal and distance.
    cpVect n = cpvnormalize(cpvperp(cpvsub(b, a))); // spocitaji vektor mezi 'b' a 'a' ->kolmy vektor -> normalizace (stejny smer, delka 1)
    cpFloat dist = cpvdot(a, n);                    // skalarni soucin normaly a bodu a - > chteji zjitit, na ktere strane od rezane cary -> pro kazdy bod budou zjistovat, na ktere strane lezi
   
    // volam dvakrat, protoze mam po uriznuti dve poloviny ( projde vsechny body a urci, jestli patri do "aktualne kontrolovane strany")
    ClipPoly(space, shape, n, dist, context->blocks, 1);
    ClipPoly(space, shape, cpvneg(n), -dist, context->blocks,-1);

    //odtsraneni puvodniho tvaru, co byl preriznuty 
    dynarray *fruit2 = context->blocks;
    for (int i = 0; i < context->blocks->size; i++)
    {
        Fruit *f = (Fruit *)fruit2->items[i];
        if (f->shape == shape)
        {
            dynarray_remove(fruit2, f);
            break;
        }
    }
    cpBody *body = cpShapeGetBody(shape);
    cpSpaceRemoveShape(space, shape);
    cpSpaceRemoveBody(space, body);
    cpShapeFree(shape);
    cpBodyFree(body);
}

void SliceQuery(cpShape *shape, cpVect point, cpVect normal, cpFloat alpha, SliceContext *context)
{
    cpVect a = context->a; // zacatek rezu
    cpVect b = context->b; // konec rezu

    if (cpShapePointQuery(shape, a, NULL) > 0.0f && cpShapePointQuery(shape, b, NULL) > 0.0f) //urci, jestli seknuti slo opravdu skrz a neskoncilo v pulce tvaru
    {
        // behem kontroly protnuti nesmim menit shapy, iteruju je - az bude bezpecne menit shapy, zavolej SliceShapePostStep
        cpSpaceAddPostStepCallback(context->space, (cpPostStepFunc)SliceShapePostStep, shape, context);
        *context->points += 1;

        //co 10 bodu novy level, maximalni rychlost tvorby ovoce je co sekundu
        if (*context->points % 10 == 0 && *context->new_fruit > 1 && *context->points != 0)
        {
            *context->level += 1;
            *context->new_fruit -= 0.5f;
        }
    }
}
