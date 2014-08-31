#ifndef GFX_H
#define GFX_H

#include <SDL.h>
#include "wire.h"

typedef struct Gfx Gfx;
struct Gfx {
	SDL_Window *window;
	SDL_Surface *surf;
	SDL_Renderer *renderer;
};

void gfx_init(Gfx *gfx);
void gfx_deinit(Gfx *gfx);
int gfx_event(Gfx *gfx);
void gfx_draw(Gfx *gfx, WireNode *ns, int n);

#endif
