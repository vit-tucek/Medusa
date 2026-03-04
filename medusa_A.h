#ifndef __MEDUSA_A_H
#define __MEDUSA_A_H

#include <complex.h>
#include "dynarray.h"

typedef double ddouble;
typedef complex <double> CComplex;


class Corruption { };

int find_circle(CComplex, CComplex, CComplex, CComplex&, ddouble&, CComplex&);

int is_homotopic (CComplex a, CComplex b, CComplex star,
		      DynArray<CComplex>& distinguished_pts,
		      CComplex& new_point);

int can_prune (CComplex a, CComplex b, CComplex c, CComplex star,
	       DynArray<CComplex>& distinguished_pts);

#endif
