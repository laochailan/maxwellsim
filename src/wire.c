#include "wire.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "constants.h"

void wire_node_init(WireNode *node, double x, double y, double sigma, int8_t type) {
	memset(node, 0, sizeof(WireNode));
	node->pos[0] = x;
	node->pos[1] = y;
	node->sigma = sigma;
	node->type = type;
}

static void reserve_neighbor(WireNode *n) {
	n->ncount++;
	if(n->ncount > n->ncapacity) {
		n->ncapacity++;
		n->ncapacity *= 2;
		n->neighbors = realloc(n->neighbors, n->ncapacity*sizeof(WireNode *));
	}
}


void wire_node_join(WireNode *a, WireNode *b) {
	reserve_neighbor(a);
	reserve_neighbor(b);

	a->neighbors[a->ncount-1] = b;
	b->neighbors[b->ncount-1] = a;
}

void wire_node_free(WireNode *node) {
	free(node->neighbors);
}

void wire_update_potential(WireNode *ns, int n) { // Uses Jacobi iterations or so
	int iter;
	double eps_max;
	int d;
	int i;
	double min;

	for(iter = 0; iter < POTENTIAL_MAX_ITER; iter++) {
		d = iter&1;

		eps_max = 0;

		for(i = 0; i < n; i++) {
			int j;
			double s1 = 0;
			double s2 = 0;
			double V = 0;
			double e;
			
			if(ns[i].ncount <= 1) // border
				continue;

			for(j = 0; j < ns[i].ncount; j++) {
				WireNode *ne = ns[i].neighbors[j];
				Vec dr;
				double r;
				dr[0] = ne->pos[0]-ns[i].pos[0];
				dr[1] = ne->pos[1]-ns[i].pos[1];
				r = dr[0]*dr[0]+dr[1]*dr[1];
				
				s1 += ne->U[d]/r;
				s2 += 1./r;
				V += sqrt(r);
			}
			
			V *= WIRE_AREA/2.;

			ns[i].U[!d] = ns[i].U[d]*0.0+1.*(s1-ns[i].rho/EPS0)/s2;

			e = fabs(ns[i].U[!d]-ns[i].U[d]);
			if(e > eps_max)
				eps_max = e;
		}
		if(eps_max < EPSILON_CONV)
			break;
	}
	//printf("iterations: %d, eps_max: %g\n", iter, eps_max);
	

	min = ns[i].U[d];
	for(i = 0; i < n; i++) {
		int j;
		double corr = 0;
		for(j = 0; j < ns[i].ncount; j++)
			corr += ns[i].neighbors[j]->U[!d];
				
		ns[i].U[d] = ns[i].U[!d]*0.8+0.2*corr/ns[i].ncount;
		if(ns[i].U[d] < min)
			min = ns[i].U[d];
	}
	
	//for(i = 0; i < n; i++) {
	//	ns[i].U[d]-= min;
	//}	

	if(d != 0) {
		for(i = 0; i < n; i++)
			ns[i].U[0] = ns[i].U[1];
	}
}



static void wire_node_update_charge(WireNode *n) {
	double drhodt = 0;
	int i;
	
	if(n->ncount <= 1) {
		return;
	}

	for(i = 0; i < n->ncount; i++) {
		WireNode *ne = n->neighbors[i];
		Vec dr;
		if(ne->ncount <= 1)
			continue;

		double U = ne->U[0]-n->U[0];
		dr[0] = n->pos[0]-ne->pos[0];
		dr[1] = n->pos[1]-ne->pos[1];

		drhodt += U/(dr[0]*dr[0]+dr[1]*dr[1]);
	}
	n->drhodt = -n->sigma*drhodt;
}

void wire_update_charge(WireNode *ns, int n, double dt) {
	int i;
	for(i = 0; i < n; i++)
		wire_node_update_charge(&ns[i]);

	for(i = 0; i < n; i++) {
		ns[i].rho += ns[i].drhodt*dt;
	}
}
