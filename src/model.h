#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include "constants.h"
#include "vec.h"

enum CellTypes {
	CT_VACUUM = 0,
	CT_METAL = 1,
	CT_METALBORDER = 2,
	CT_BORDERCOND = 3
};

typedef struct Model Model;
struct Model {
	int w; // width of domain
	int h; // height of domain

	double *phi[3];
	double *Ax[3];
	double *Ay[3];
	int tp;

	double *Ex;
	double *Ey;

	double *rho; // charge density
	double *jx; // current density
	double *jy; // current density

	int8_t *celltype;
	double *sigma; // conductivity

	double *damping;
};

void model_allocate(Model *m, int x, int y);
void model_static_field(Model *m);
void model_free(Model *m);

void model_update(Model *m, double dt);

#endif
