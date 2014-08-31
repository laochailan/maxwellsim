#ifndef WIRE_H
#define WIRE_H

#include "vec.h"
#include <stdint.h>

enum Type {
	WNTYPE_NONE = 0,
	WNTYPE_BORDER = 1
};

typedef struct WireNode WireNode;
struct WireNode {
	Vec pos;
	
	double drhodt, rho; // charge density C/m
	double U[2]; // potential V/m [old/new]
	double sigma; // electrical conductivity m/Ohm

	uint8_t type;
	
	int ncount;
	int ncapacity;
	WireNode **neighbors;
};

void wire_node_init(WireNode *node, double x, double y, double sigma, int8_t type);
void wire_node_join(WireNode *a, WireNode *b);
void wire_node_free(WireNode *node);
void wire_update_charge(WireNode *ns, int n, double dt);
void wire_update_potential(WireNode *ns, int n);
#endif
