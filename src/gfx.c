#include "gfx.h"

#include <stdlib.h>
#include "model.h"

enum {
	SCREENW = 600,
	SCREENH = 600
};


void gfx_init(Gfx *gfx, int w, int h) {
	SDL_Init(SDL_INIT_VIDEO);
	float r = w/(float)h;
	int size = 700;
	gfx->window = SDL_CreateWindow("maxwellsim", 0, 0, size, size/r, SDL_WINDOW_SHOWN);
	gfx->renderer = SDL_CreateRenderer(gfx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	gfx->tex = SDL_CreateTexture(gfx->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, w, h);
}

void gfx_deinit(Gfx *gfx) {
	SDL_DestroyTexture(gfx->tex);
	SDL_DestroyRenderer(gfx->renderer);
	SDL_DestroyWindow(gfx->window);
	SDL_Quit();
}

static void val2hue(uint8_t *p, double val) {

	double m = val-((int)(val));
	double q = 255*(1-m);
	double t = 255*(m);
	
	if(isnan(val)) {
		p[0] = 0;
		p[1] = 0;
		p[2] = 0;
	}
	switch(((int)val)%6) {
	case 0:
		p[0] = 255;
		p[1] = t;
		p[2] = 0;
		break;
	case 1:
		p[0] = q;
		p[1] = 255;
		p[2] = 0;
		break;
	case 2:
		p[0] = 0;
		p[1] = 255;
		p[2] = t;
		break;
	case 3:
		p[0] = 0;
		p[1] = q;
		p[2] = 255;
		break;
	case 4:
		p[0] = t;
		p[1] = 0;
		p[2] = 255;
		break;
	case 5:
		p[0] = 255;
		p[1] = 0;
		p[2] = q;
		break;
	}
}

static int clampround(double x, int w) {
	int c = x+0.5;
	if(c > w)
		c = w;
	if(c < 0)
		c = 0;
	return c;
}

void gfx_draw(Gfx *gfx, Model *m) {
	uint8_t *pix;
	int pitch;
	SDL_LockTexture(gfx->tex, 0, (void **)&pix, &pitch);
	for(int x = 0; x < m->w; x++) {
		for(int y = 0; y < m->h; y++) {
			int ct = m->celltype[y*m->w+x];
			uint8_t *p = &pix[3*(y*pitch/3+x)];
			p[0] = 0;
			p[1] = 0;
			p[2] = 0;

			if(ct == CT_METAL || ct == CT_BORDERCOND) {
				p[0] = 100;
				p[1] = 100;
				p[2] = 100;
			} else if(ct == CT_METALBORDER) {
				p[0] = 150;
				p[1] = 150;
				p[2] = 150;
			}
		
			Vec j, E;
		        j.x = m->Ax[m->tp][y*m->w+x];
		        j.y = m->Ay[m->tp][y*m->w+x];

			E.x = m->Ex[y*m->w+x];
			E.y = m->Ey[y*m->w+x];

			p[0] = clampround(p[0]+2000*sqrt(E.x*E.x+E.y*E.y), 255);
			//p[0] = clampround(p[0]+200*m->phi[m->tp][y*m->w+x], 255);
			p[1] = clampround(p[1]+200*m->rho[y*m->w+x], 255);
			p[2] = clampround(p[2]+200*sqrt(j.x*j.x + j.y*j.y), 255);
		}
	}

	SDL_UnlockTexture(gfx->tex);
	SDL_RenderCopy(gfx->renderer, gfx->tex, 0, 0);

	SDL_RenderPresent(gfx->renderer);
}

int gfx_event(Gfx *gfx) {
	SDL_Event e;
	while(SDL_PollEvent(&e) != 0) {
		if(e.type == SDL_QUIT)
			return -1;
	}

	return 0;
}

