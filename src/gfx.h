#ifndef GFX_H
#define GFX_H

#include <SDL.h>
#include "model.h"

typedef struct Gfx Gfx;
struct Gfx {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *tex;
};

void gfx_init(Gfx *gfx, int w, int h);
void gfx_deinit(Gfx *gfx);
int gfx_event(Gfx *gfx);
void gfx_draw(Gfx *gfx, Model *m);

#endif
