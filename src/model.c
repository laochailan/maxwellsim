#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "model.h"

static void model_update_j(Model *m, double dt);

static void model_init_damping(double *damping, int w, int h) { // damping factor at the edges to absorb waves
	int x, y;

	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			double xf = fabs((x-w/2)/(double)w);
			double yf = fabs((y-h/2)/(double)h);
			
			double t = max(xf,yf);
			double df = 0;
			if(t > 0.40)
				df = 0.1*(t-0.30)*(t-0.30)/0.1;
			
			damping[y*w+x] = df;
		}
	}
}

void model_allocate(Model *m, int w, int h) {
	int i;
	int size = w*h;

	memset(m, 0, sizeof(Model));

	m->w = w;
	m->h = h;

	for(i = 0; i < 3; i++) {
		m->phi[i] = calloc(size, sizeof(double));
		m->Ax[i] = calloc(size, sizeof(double));
		m->Ay[i] = calloc(size, sizeof(double));
		
		memset(m->phi[i],0,size*sizeof(double));
		memset(m->Ax[i],0,size*sizeof(double));
		memset(m->Ay[i],0,size*sizeof(double));
	}

	m->rho = calloc(size, sizeof(double));
	m->jx = calloc(size, sizeof(double));
	m->jy = calloc(size, sizeof(double));
	m->Ex = calloc(size, sizeof(double));
	m->Ey = calloc(size, sizeof(double));
	m->celltype = calloc(size, sizeof(int8_t));
	m->sigma = calloc(size, sizeof(double));
	m->damping = calloc(size, sizeof(double));
	
	memset(m->rho,0,size*sizeof(double));
	memset(m->jx,0,size*sizeof(double));
	memset(m->jy,0,size*sizeof(double));
	memset(m->Ex,0,size*sizeof(double));
	memset(m->Ey,0,size*sizeof(double));
	memset(m->celltype,0,size*sizeof(int8_t));
	memset(m->sigma,0,size*sizeof(double));
	memset(m->damping,0,size*sizeof(double));

	model_init_damping(m->damping, w, h);
}

static void model_solve_poisson(double *data, double *tmp, double *src, int8_t *celltype, int w, int h) {
	int POISSON_MAX_ITER = 10000;
	int iter;
	double eps_max;
	int d;
	double *U[2];
	
	double t = cos(M_PI/w)+cos(M_PI/h);
	double wo = (8-sqrt(64-16*t*t))/(t*t);

	U[0] = data;
	U[1] = tmp;
	

	for(iter = 0; iter < POISSON_MAX_ITER; iter++) {
		int x, y;
		d = iter&1;

		eps_max = 0;

		for(y = 0; y < h; y++) {
			for(x = 0; x < w; x++) {
				double r;
				
				if(celltype[y*w+x] == CT_BORDERCOND)
					continue;

				r = U[!d][y*w+x-1]+U[d][y*w+x+1]
				   +U[!d][(y-1)*w+x]+U[d][(y+1)*w+x]
				   +DX*DY*src[y*w+x];
				
				U[!d][y*w+x] = (1-wo)*U[d][y*w+x]+wo*r/4;

				double e = fabs(U[!d][y*w+x]-U[d][y*w+x]);
				eps_max += e*e;
			}

			
		}
		
		if(sqrt(eps_max/w/h) < EPSILON_CONV)
			break;
	}

	if(d == 0)
		memcpy(data, U[1], w*h*sizeof(double));
	printf("iterations: %d, eps_max: %g\n", iter, sqrt(eps_max/w/h));
}

void model_static_field(Model *m) {
	assert(fabs(DX-DY) < 1e-8); // only square grids for now
	printf("Calculate starting conditions ...\n");
	model_solve_poisson(m->phi[1], m->phi[0], m->rho, m->celltype, m->w, m->h);
	memcpy(m->phi[0], m->phi[1], m->w*m->h*sizeof(double));
	memcpy(m->phi[2], m->phi[1], m->w*m->h*sizeof(double));
	model_update_j(m, 0.01);
	model_solve_poisson(m->Ax[1], m->Ax[0], m->jx, m->celltype, m->w, m->h);
	memcpy(m->Ax[0], m->Ax[1], m->w*m->h*sizeof(double));
	memcpy(m->Ax[2], m->Ax[1], m->w*m->h*sizeof(double));
	model_solve_poisson(m->Ay[1], m->Ay[0], m->jy, m->celltype, m->w, m->h);
	memcpy(m->Ay[0], m->Ay[1], m->w*m->h*sizeof(double));
	memcpy(m->Ay[2], m->Ay[1], m->w*m->h*sizeof(double));
}

void model_free(Model *m) {
	int i;
	for(i = 0; i < 3; i++) {
		free(m->phi[i]);
		free(m->Ax[i]);
		free(m->Ay[i]);
	}
	free(m->rho);
	free(m->jx);
	free(m->jy);
	free(m->Ex);
	free(m->Ey);
	free(m->celltype);
	free(m->sigma);
}


static void model_update_j(Model *m, double dt) {
	int x,y;
	int tp1 = m->tp;

	for(y = 1; y < m->h-2; y++) {
		for(x = 1; x < m->w-2; x++) {
			//int ct = m->celltype[y*m->w+x];
			int i, inmetx, inmety;
			//if(ct != CT_BORDERCOND) {
				m->Ex[y*m->w+x] = -(m->phi[tp1][y*m->w+x+1]-m->phi[tp1][y*m->w+x])/DX;
				m->Ey[y*m->w+x] = -(m->phi[tp1][(y+1)*m->w+x]-m->phi[tp1][y*m->w+x])/DY;
				
				m->Ex[y*m->w+x] -= (m->Ax[m->tp][y*m->w+x]-m->Ax[(m->tp+1)%3][y*m->w+x])/(2*dt*C);
				m->Ey[y*m->w+x] -= (m->Ay[m->tp][y*m->w+x]-m->Ay[(m->tp+1)%3][y*m->w+x])/(2*dt*C);
			//}
			
			inmetx = 1;
			inmety = 1;
			for(i = 0; i <= 1; i++) {
				inmetx = inmetx && (m->celltype[y*m->w+x+i] == CT_METAL
					|| m->celltype[y*m->w+x+i] != CT_VACUUM);
				inmety = inmety && (m->celltype[(y+i)*m->w+x] == CT_METAL
					|| m->celltype[(y+i)*m->w+x] != CT_VACUUM);
								
			}

			if(inmetx)
				m->jx[y*m->w+x] = m->Ex[y*m->w+x]*m->sigma[y*m->w+x]/C;
			if(inmety)
				m->jy[y*m->w+x] = m->Ey[y*m->w+x]*m->sigma[y*m->w+x]/C;
		}
	}
}

void model_update_wave(double *data[3], double *src, int8_t *celltype, double *damping,
                       int tp, double dt, int x1, int x2, int y1, int y2, int w) {
	int x, y;
	
	int tm1 = (tp+1)%3;
	int t0 = (tp+2)%3;
	int tp1 = tp;

	double cf = dt*dt*C*C;

	for(y = y1; y < y2; y++) {
		for(x = x1; x < x2; x++) {
			int idx = y*w+x;
			if(celltype[idx] == CT_BORDERCOND)
				continue;

			double lap = data[t0][idx-1]+data[t0][idx+1]
			             +data[t0][idx-w]+data[t0][idx+w]
				     -4*data[t0][idx];
			double drdt = data[t0][idx]-data[tm1][idx];
			lap /= DX*DY;
			lap += src[idx];

			data[tp1][idx] = data[t0][idx]+drdt+cf*(lap-damping[idx]/dt*drdt);
		}
	}

}

void model_update(Model *m, double dt) {
	static int t = 0;
	int x, y;
	assert(C*dt < DX/1.45);
	assert(dt > 0);
	
	model_update_wave(m->phi, m->rho, m->celltype, m->damping, m->tp, dt, 0, m->w, 0, m->h, m->w);
	model_update_wave(m->Ax, m->jx, m->celltype, m->damping, m->tp, dt, 1, m->w-2, 1, m->h-2, m->w);
	model_update_wave(m->Ay, m->jy, m->celltype, m->damping, m->tp, dt, 1, m->w-2, 1, m->h-2, m->w);

	model_update_j(m, dt);

	for(y = 0; y < m->h; y++) {
		for(x = 0; x < m->w; x++) {
			int ct = m->celltype[y*m->w+x];
			if(ct == CT_BORDERCOND)
				continue;
	
			double divj = (m->jx[y*m->w+x]-m->jx[y*m->w+x-1])/DX
				+ (m->jy[y*m->w+x]-m->jy[(y-1)*m->w+x])/DY;

			m->rho[y*m->w+x] -= dt*divj*C;
		}
	}

	t++;

	m->tp = (m->tp+1)%3;
}
