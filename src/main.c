#include "model.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL.h>

#include "gfx.h"
#include "constants.h"
#include "loadmap.h"

int main(int argc, char **argv) {
	Gfx gfx;
	Model model;
	
	if(argc != 2) {
		printf("Invalid count of arguments!\nUsage %s FILENAME\n", argv[0]);
		return 1;
	}

	if(load_map(argv[1], &model))
		return 1;
	model_static_field(&model);
	gfx_init(&gfx, model.w, model.h);
	double t = SDL_GetTicks()/1000.-0.016;
	while(1) {
		int i;
		int nsteps = 5;

		if(gfx_event(&gfx) < 0)
			break;

		double dt = SDL_GetTicks()/1000.-t;
		dt = 0.06;
		if(nsteps < dt*C/DX*5) {
			nsteps = dt*C/DX*5+0.5;
			printf("nsteps %d\n", nsteps);
			if(nsteps > 30)
				nsteps = 30;
		}

		for(i = 0; i < nsteps; i++)
			model_update(&model, dt/nsteps);
		
		double rho = 0;
		int x, y;
		for(x = 0; x < model.w; x++) {
			for(y = 0; y < model.h; y++) {
				rho += model.rho[y*model.w+x];
			}
		}		
		double tdt = SDL_GetTicks()/1000.-t;
		t = SDL_GetTicks()/1000.;
		printf("%f fps aval = %g\n", 1./tdt, model.phi[model.tp][model.h/2*model.w+model.w*8/10-2]);
		gfx_draw(&gfx, &model);
	}

	gfx_deinit(&gfx);
	model_free(&model);

	return 0;
}

