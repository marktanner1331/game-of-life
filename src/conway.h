// greet.h
#ifndef CONWAY
#define CONWAY

#include <windows.h>

typedef struct
{
    unsigned char width;
    unsigned char height;
    unsigned char *currentState;
    unsigned char *previousState;
} conway_GameOfLife;

typedef struct
{
    HDC backDC;
    HBITMAP backBitmap;
    HBITMAP oldBitmap;
    unsigned int bufferWidth;
    unsigned int bufferHeight;
} conway_RenderContext;

conway_GameOfLife *conway_initialize(unsigned char width, unsigned char height);

void conway_free(conway_GameOfLife *game);

int conway_initGliderGun(conway_GameOfLife *game);

void conway_tick(conway_GameOfLife *game);

void conway_render(conway_RenderContext *ctx, HDC hdc, conway_GameOfLife *game);

#endif