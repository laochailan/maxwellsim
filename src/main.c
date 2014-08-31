#include "wire.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "gfx.h"

#define N 200

int main(int argc, char **argv) {
	Gfx gfx;
	WireNode nodes[N];
	int i;

	for(i = 0; i < N; i++) {
		wire_node_init(&nodes[i], 1000*cos(2.*M_PI/N*i), 1000*sin(2.*M_PI/N*i), 1e7, 0);
		nodes[i].rho = -10e-19*exp(-(N/2-i)*(N/2-i));
		if(i > 0)
			wire_node_join(&nodes[i],&nodes[i-1]);
	}

	//wire_node_join(&nodes[0], &nodes[N-1]);

	nodes[0].U[0] = 0.01;
	//nodes[0].type = WNTYPE_BORDER;
	
	//nodes[N-1].U[0] = 0.005;
	//nodes[N-1].type = WNTYPE_BORDER;
	
	gfx_init(&gfx);
	while(1) {
		int t;
		double rs = 0.;
		if(gfx_event(&gfx) < 0)
			break;
		t = clock();
		while(clock()-t < CLOCKS_PER_SEC/40.) {
			wire_update_potential(nodes, N);
			wire_update_charge(nodes, N, 0.16e-21);
		}
		gfx_draw(&gfx, nodes, N);
		
		for(i = 0; i < N; i++)
			rs += nodes[i].rho;
		printf("rhosum: %g\n", rs);
		printf("%g\n", nodes[1].rho);
		printf("%g\n", nodes[N-2].rho);
	/*	for(i = 0; i < N; i++)
			printf("%g ", nodes[i].U[0]);
		printf("\n");*/
		//for(i = 0; i < N; i++)
			//printf("%g ", nodes[i].rho);
		//printf("\n");

	}

	gfx_deinit(&gfx);
	for(i = 0; i < N; i++) {
		wire_node_free(&nodes[i]);
	}
	return 0;
}
