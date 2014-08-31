#include "gfx.h"

#include <stdlib.h>

enum {
	SCREENW = 600,
	SCREENH = 600
};

void gfx_init(Gfx *gfx) {
	int rc;
	
	rc = SDL_Init(SDL_INIT_VIDEO);
	if(rc < 0) {
		printf("SDL could not be initialized: %s\n", SDL_GetError());
		exit(1);
	}

	gfx->window = SDL_CreateWindow("maxwellsim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, SDL_WINDOW_SHOWN);

	if(gfx->window == NULL) {
		printf("Window not be initialized: %s\n", SDL_GetError());
		exit(1);
	}

	gfx->renderer = SDL_CreateRenderer(gfx->window, -1, SDL_RENDERER_ACCELERATED);
}

void gfx_deinit(Gfx *gfx) {
	SDL_DestroyRenderer(gfx->renderer);
	SDL_DestroyWindow(gfx->window);
	SDL_Quit();
}

int gfx_event(Gfx *gfx) {
	SDL_Event e;
	while(SDL_PollEvent(&e) != 0) {
		if(e.type == SDL_QUIT)
			return -1;
	}

	return 0;
}

void gfx_draw(Gfx *gfx, WireNode *ns, int n) {
	SDL_Point *pts;
	int i;
	double scale = fabs(ns[0].U[0]);
	SDL_RenderClear(gfx->renderer);
	
	pts = calloc(n, sizeof(SDL_Point));

	for(i = 0; i < n; i++) {
		if(fabs(ns[i].U[0]) > scale)
			scale = fabs(ns[i].U[0]);
	}
	if(scale == 0.)
		scale = 1.;	

	for(i = 0; i < n; i++) {
		pts[i].x = SCREENW/10+SCREENW*0.8/(n-1)*i;
		pts[i].y = SCREENH/3-ns[i].U[0]/scale*100;
	}
	
	SDL_RenderDrawLines(gfx->renderer, pts, n);

	for(i = 0; i < n; i++) {
		pts[i].y = SCREENH*2/3-ns[i].rho*1e21;
	}
	
	SDL_RenderDrawLines(gfx->renderer, pts, n);

	free(pts);
	SDL_RenderPresent(gfx->renderer);
}
