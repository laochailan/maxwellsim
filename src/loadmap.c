#include "loadmap.h"
#include "model.h"

#include <png.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int load_png(FILE *f, uint8_t **bytes, int *w, int *h) {
	uint8_t header[8];
	int i;
	fread(header, 1, sizeof(header), f);
	if(png_sig_cmp(header, 0, sizeof(header))) {
		printf("File does not seem to be png.\n");
		return 1;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		return 1;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 1;
	}
	png_infop end_info = png_create_info_struct(png_ptr);
	if(!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 1;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return 1;
	}
	
	memset(bytes, 0, 3**w**h);

	png_init_io(png_ptr, f);
	png_set_sig_bytes(png_ptr, sizeof(header));

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, NULL);

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

	*w = png_get_image_width(png_ptr, info_ptr);
	*h = png_get_image_height(png_ptr, info_ptr);
	
	*bytes = malloc(*w**h*3);
	for(i = 0; i < *h; i++) {
		memcpy(*bytes + 3*i**w, row_pointers[i], *w*3);
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	return 0;
}

static void parse_celltype(uint8_t *bytes, int8_t *celltype, double *sigma, int w, int h) {
	int x, y;
	
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			int8_t t = CT_VACUUM;

			if(bytes[3*(y*w+x)] > 0) {
				t = CT_METAL;
				sigma[y*w+x] = 0.8;
			}
			if(bytes[3*(y*w+x)+1] != 128) {
				t = CT_BORDERCOND;
				sigma[y*w+x] = 0.8;
			}

			if(x == 0 || x == w-1 || y == 0 || y == h-1)
				t = CT_BORDERCOND;	

			celltype[y*w+x] = t;
		}
	}
	
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			int i;
			if(celltype[y*w+x] != CT_METAL)
				continue;
			for(i = -1; i <= 1; i++) {
				if(celltype[y*w+x+i] == CT_VACUUM || celltype[(y+i)*w+x] == CT_VACUUM) {
					celltype[y*w+x] = CT_METALBORDER;
					break;
				}
			}
		}
	}
}

static void parse_phirho(uint8_t *bytes, double **phi, double *rho, int w, int h) {
	int x, y;
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			double p = (bytes[3*(y*w+x)+1]-128)/128.*0.7;
			phi[0][y*w+x] = p;
			phi[1][y*w+x] = p;
			phi[2][y*w+x] = p;

			rho[y*w+x] = bytes[3*(y*w+x)+2]/256.*0.1;
			
		}
	}
}

int load_map(char *filename, Model *m) {
	FILE *f = fopen(filename, "rb");
	int rc;
	int w, h;
	uint8_t *bytes;

	if(!f) {
		fprintf(stderr, "LoadMap Error: Could not open '%s': %s\n", filename, strerror(errno));
		return 1;
	}
	
	rc = load_png(f, &bytes, &w, &h);
	fclose(f);
	if(rc) {
		fprintf(stderr, "LoadMap Error: Failed to load '%s'\n", filename);
		return 1;
	}

	model_allocate(m, w, h);
	parse_celltype(bytes, m->celltype, m->sigma, w, h);
	parse_phirho(bytes, m->phi, m->rho, w, h);
	
	free(bytes);
	return 0;
}
