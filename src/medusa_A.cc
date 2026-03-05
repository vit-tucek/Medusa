/* Author: Kuan Ju Liu
 * Last Modified: 30-Jul-98
 */

#include <complex.h>
#include "dynarray.C"
#include "medusa_A.h"

typedef complex<double> Complex;


//#define DEBUG_SUB	/* Debug subroutines (l_i_h, l_i_c, etc.) */
//#define DEBUG_FC	/* Debug find_circle ()                   */
//#define DEBUG_FI	/* Debug find_intersections ()            */
//#define DEBUG_FP	/* Debug Failure pts.                     */

const ddouble epsilon = 1e-15;
const ddouble EPSILON = 1e-15;
const ddouble infinity = 1e100;
const ddouble MEDUSA_INFINITY = 1e100;

//----------------------------------------------------------------------

/*
 * numerical_equal --
 * 
 * Return 1 iff two CComplex numbers seem to be the same 
 *
 */

int numerical_equal(CComplex x, CComplex y)
{
	ddouble magnitude = sqrt (norm (x) + norm (y));

	if (magnitude <= EPSILON)
		return (1);

	return (abs (x-y) / magnitude < EPSILON);
}

/*
 * rotate_by --
 *
 * Returns the first CComplex number, a, rotated by
 * theta.
 */
CComplex		rotate_by (CComplex a, ddouble theta)
{
	CComplex z;
	
	z = a * polar(1.0, theta);
	
	return (z);
}
/* End of rotate_by */



/*
 * lies_in_sector --
 *
 * This function takes the two given points a, b,
 * and a center point x, and returns 1 if the point
 * z lies in the sector axb, but not on the boundary.
 * Note, assumes that Arg(a) Arg(b), with respect
 * to the origin.
 */
int		lies_in_sector (CComplex a, CComplex b, CComplex x, CComplex z)
{
	CComplex a_prime, z_prime;

	a_prime = (a-x) / (b-x) * CComplex (polar (1.0, (double)M_PI));
	z_prime = (z-x) / (b-x) * CComplex (polar (1.0, (double)M_PI));

	if (arg (z_prime) < arg (a_prime)
	    && arg (z_prime) > -M_PI)
		return (1);

	return (0);
}
/* End of lies_in_sector */



/*
 * not_lies_in_sector --
 *
 * Returns 1 if the point is outside the sector
 * defined by a, b, and the center point x.
 */
int not_lies_in_sector (CComplex a, CComplex b, CComplex x, CComplex z)
{
	CComplex a_prime, z_prime;

	a_prime = (a-x) / (b-x) * CComplex (polar (1.0, (double)M_PI));
	z_prime = (z-x) / (b-x) * CComplex (polar (1.0, (double)M_PI));

	if (arg (z_prime) > arg (a_prime)
	    && arg (z_prime) < M_PI)
		return (1);

	return (0);
}
/* End of not_lies_in_sector */



/*
 * mobius_to_line --
 *
 * Sends the circle through a, b, and star to the real
 * axis, sending a to 0, b to 1, and star to infinity.
 * Does this to the input z.
 */
CComplex mobius_to_line (CComplex z, CComplex a, CComplex b, CComplex star)
{
	if (numerical_equal (z, star))
		return (MEDUSA_INFINITY);

	return ((z-a)/(z-star) * (b-star)/(b-a));
}
/* End of mobius_to_line*/




/*
 * lies_in_circle --
 *
 * Given a circle by three points a, b, and star, 
 * determine if the given point z is inside or on it.
 */
int	lies_in_circle (CComplex z, CComplex a, CComplex b, CComplex star, ddouble up_or_down)
{
	CComplex z_prime = mobius_to_line (z, a, b, star);
	/*
	 cout << "Which? z, a, b, star =" << z << a << b << star << endl; cout.flush ();
	 cout << "Where? " << imag (z_prime) << up_or_down << endl; cout.flush ();
	 cout << "What? (z-a)/(z-star), (b-star)/(b-a) = " << (z-a)/(z-star) << (b-star)/(b-a) << endl; cout.flush ();
	 */
	if (imag (z_prime) * up_or_down >= 0)
		return (1);

#ifdef DEBUG_SUB
	cout << "Not in circle" << endl; cout.flush ();
#endif

	return 0;
}
/* End of lies_in_circle */



/*
 * not_lies_in_circle --
 *
 * Given a circle by three points a, b, and star, 
 * determine if the given point z is outside or on it.
 */
int	not_lies_in_circle (CComplex z, CComplex a, CComplex b, CComplex star, ddouble up_or_down)
{
	CComplex z_prime = mobius_to_line (z, a, b, star);
	/*
	cout << "Which? z, a, b, star =" << z << a << b << star << endl; cout.flush ();
	cout << "Where? " << imag (z_prime) << up_or_down << endl; cout.flush ();
	cout << "What? (z-a)/(z-star), (b-star)/(b-a) = " << (z-a)/(z-star) << (b-star)/(b-a) << endl; cout.flush ();
	*/
	if (imag (z_prime) * up_or_down <= 0)
		return (1);

#ifdef DEBUG_SUB
	cout << "In circle" << endl; cout.flush ();
#endif

	return 0;
}
/* End of not_lies_in_circle */




/*
 * lies_in_hyperbola --
 *
 * Takes one (CComplex) point a on the hyperbola which has 
 * the real axes as its asymptotes. It tells us whether
 * the given point z is inside this hyperbola or not.
 * NOTE: It also tells you if z lies in the same half of
 * the hyperbola as a. Also tells if z lies on the hyperbolic
 * segment between a and b. BE CAREFUL!
 */
int	lies_in_hyperbola (CComplex a, CComplex b, CComplex z)
{
	ddouble hyp_const = (real (a)*imag (a) < real (b)*imag (b)) ? real (a)*imag (a): real (b)*imag (b);

#ifdef DEBUG_SUB
	if (numerical_equal (a, 0)) {
		cout << "bad a in lies_in_hyperbola" << a << endl;
		exit (0);
	}
#endif
	if (numerical_equal (z, a) || numerical_equal (z, b))
		return (0);

	if (real (z) * imag (z) >= hyp_const) {
		if (numerical_equal (z, 0))
			return (1);
		if (abs (arg (a / z)) <= M_PI / 2)
			return (1);
	}
	//if ((real (z) * imag (z) == real (a) * imag (a))
	//   && lies_in_sector (b, a, 0, z))
	//	return (1);

#ifdef DEBUG_SUB
	cout << "Not in hyperbola" << endl; cout.flush (); 
#endif
	return (0);
}
/* End of lies_in_hyperbola */



/*
 * not_lies_in_hyperbola --
 *
 * Takes one (CComplex) point a on the hyperbola which has 
 * the real axes as its asymptotes. It tells us whether
 * the given point z is outside (or on) this hyperbola or not.
 * NOTE: It first tells you if z lies in the other half of
 * the hyperbola as a. Also tells if z lies on the hyperbolic
 * segment between a and b. BE CAREFUL!
 */
int	not_lies_in_hyperbola (CComplex a, CComplex b, CComplex z)
{
	ddouble hyp_const = (real (a)*imag (a) < real (b)*imag (b)) ? real (a)*imag (a): real (b)*imag (b);

#ifdef DEBUG_SUB
	if (numerical_equal (a, 0)) {
		cout << "bad a in lies_in_hyperbola" << a << endl;
		exit (0);
	}
#endif
	if (numerical_equal (z, a) || numerical_equal (z, b))
		return (0);

	if (real (z) * imag (z) <= hyp_const)
		return (1);
	if (numerical_equal (z, 0))
		return (1);
	if (abs (arg (a / z)) >= M_PI / 2)
		return (1); 
	//if ((real (z) * imag (z) == hyp_const)
	//   && lies_in_sector (b, a, 0, z))
	//	return (1);

#ifdef DEBUG_SUB
	cout << "In hyperbola" << endl; cout.flush ();
#endif
	return (0);
}
/* End of not_lies_in_hyperbola */




/*
 * find_circle --
 *
 * Takes three points a, b, and star, and returns the
 * center point and radius of the circle. Returns 0
 * if the circle is degenerate (i.e., a line).
 */
int	find_circle (CComplex a, CComplex b, CComplex star,
		     CComplex& center, ddouble& radius, CComplex& average)
{
	ddouble a1 = real (a);
	ddouble a2 = imag (a);
	ddouble b1 = real (b);
	ddouble b2 = imag (b);
	ddouble c1 = real (star);
	ddouble c2 = imag (star);
	ddouble c, d, e;

	//if (norm (a) >= MEDUSA_INFINITY
	//    || norm (b) >= MEDUSA_INFINITY
	//    || norm (star) >= MEDUSA_INFINITY)
	//	return (0);

	//if (abs (imag ((star-a)/(b-a))) <= sqrt (EPSILON))
	//	return (0);

	if (abs (a1*b2-a1*c2-a2*b1+a2*c1-c1*b2+c2*b1) >= EPSILON) {
		c = -(-a2*b1*b1+c2*b1*b1-c2*a2*a2-a2*b2*b2+c2*c2*a2-c1*c1*b2+a2*c1*c1
		      -a1*a1*c2-b2*c2*c2+a1*a1*b2+a2*a2*b2+b2*b2*c2)/(a1*b2-a1*c2-a2*b1
								      +a2*c1-c1*b2+c2*b1);
		d = (a1*a1*b1-a1*a1*c1+a2*a2*b1-a2*a2*c1-a1*b1*b1-a1*b2*b2+a1*c1*c1
		     +a1*c2*c2-c1*c1*b1-c2*c2*b1+c1*b1*b1+c1*b2*b2)/(a1*b2-a1*c2-a2*b1
								     +a2*c1-c1*b2+c2*b1);
		e = (c2*a2*a2*b1-b1*b1*a1*c2+b1*b1*a2*c1-c2*c2*a2*b1-c1*a1*a1*b2
		     +a1*a1*c2*b1-b2*b2*a1*c2+b2*b2*a2*c1-b2*a2*a2*c1+b2*a1*c1*c1
		     +b2*a1*c2*c2-c1*c1*a2*b1)/(a1*b2-a1*c2-a2*b1+a2*c1-c1*b2+c2*b1);
		center = CComplex (- c/2, - d/2);
		radius = abs (center - a);
	} else {
		center = MEDUSA_INFINITY;
		radius = MEDUSA_INFINITY;
	}

	average = (a + b + star) / 3;

	if (abs (center) >= MEDUSA_INFINITY) {
#ifdef DEBUG_FC
		cout << "Center greater than MEDUSA_INFINITY" << center << a << b << star << endl; cout.flush ();
#endif
	}
	return (1);

	return (0);

}
/* End find_circle */




/*
 * group --
 *
 * Groups the point a with the point that lies on
 * the same half of the hyperbola. That is, it 
 * interchanges p1 and p2 if the angle formed by 
 * a, the origin, and p1 is greater than Pi.
 */
void	group (CComplex a, CComplex& p1, CComplex& p2)
{
	CComplex p_temp;

	if (abs (arg (a / p1)) > M_PI / 2) {
		p_temp = p1;
		p1 = p2;
		p2 = p_temp;
	}
}

/* End of group */




/*
 * get_midpoint --
 *
 * Returns a point on the hyperbola between
 * a and b.
 */
CComplex get_midpoint (CComplex a, CComplex b)
{
	ddouble a1 = real (a);
	ddouble a2 = imag (a);
	ddouble b1 = real (b);
	ddouble b2 = imag (b);
	CComplex z;
	ddouble hyp_const = (a1 * a2 < b1 * b2) ? a1 * a2: b1 * b2;

	/* Gets a point z on the hyperbola between a and b */
	if (abs(a1-b1) > abs(a2-b2))
		z = CComplex ((a1 + b1) / 2, 2 * hyp_const / (a1 + b1));
	else 
		z = CComplex (2 * hyp_const / (a2 + b2), (a2 + b2)/2);
	return (z);
}
/* End get_midpoint */





/*
 * bulge_or_dip --
 *
 * Tests whether the hyperbolic segment between 
 * a_prime and b_prime (previously ordered) is a
 * bulge (1) or dip (0) as defined above in
 * old_bulge_or_dip. It assumes that lista_pts
 * has all the intersection points, properly
 * ordered, and with multiplicities fully expressed.
 */
int     bulge_or_dip (CComplex a_prime, CComplex b_prime, DynArray<CComplex>& lista_pts)
{
	if (lista_pts.maxassigned () == 2)
		return (0);
	
	if (numerical_equal (lista_pts [2], a_prime)
	    && numerical_equal (lista_pts [3], b_prime))
		return (1);
	
	return (0);
}




/* 
 * order_points --
 *
 * Orders the given points so that their
 * arguments are increasing.
 */
void	order_points (DynArray<CComplex>& points)
{
	int i, j;
	int length = points.maxassigned ();
	CComplex temp;

	for (i = 1; i < length; i++)
		for (j = 1; j <= length - i; j++)
			if (arg(points [j]) > arg(points [j+1])) {
				temp = points [j+1];
				points.assign (points [j], j+1);
				points.assign (temp, j);
			}
			
#ifdef DEBUG_SUB
	cout << "In order_points: [";
	for (i = 1; i <= length; i++)
		cout << points [i] << ", ";
	cout << "]" << endl; cout.flush ();
#endif
}
/* End of order_points */




/*
 * asymptote_rotate_angle --
 *
 * Returns the vector which through multiplication,
 * will rotate the hyperbola to one with asymptotes
 * lying on the real axis. The points a and b will
 * lie in the first quadrant (not necessary).
 */
CComplex asymptote_rotate_angle (CComplex a, CComplex b)
{
	ddouble a1 = real (a);
	ddouble a2 = imag (a);
	ddouble b1 = real (b);
	ddouble b2 = imag (b);
	CComplex z;
	CComplex up = CComplex (0, 1);
	CComplex right = CComplex (1, 0);
	int i;
	CComplex temp = (numerical_equal (a, 0)) ? b : a;

	/* Start of anachronistic code */
	//	z = CComplex (a1*a1 - a2*a2 + b2*b2 - b1*b1, 2*(a1*a2 - b1*b2));

	//	z = sqrt (1 / abs (z) * z);

	//	while (!lies_in_sector (up, right, 0 , z * a))
	//		z *= up;
	/* End of anachronistic code */

	z = conj (sqrt (a*a-b*b));
	z = z / abs (z);

	i = 0;
	while (real (temp*z) < 0 || imag (temp*z) <= 0) {
		z *= up;
		if (i >= 4) {
			//cout << "Awww, hell. Temp = " << temp << endl; cout.flush ();
			throw Corruption ();
		}
		i++;
	}

	return (z);
}
/* End of asymptote_rotate_angle */




/*
 * find_intersections --
 *
 * Find the points of intersection of the hyperbola
 * through a and b, and the circle through a, b, 
 * and star. Returns the number of intersection 
 * points.
 */
int	find_intersections (CComplex a, CComplex b, CComplex star,
			    DynArray<CComplex>& intersection_pts,
			    DynArray<CComplex>& lista_pts)
{
	ddouble a1 = real (a);
	ddouble a2 = imag (a);
	ddouble b1 = real (b);
	ddouble b2 = imag (b);
	ddouble c1 = real (star);
	ddouble c2 = imag (star);
	ddouble discriminant, discriminanty;
	ddouble x, y, y1, y2;
	CComplex root;
	ddouble hyp_const = (a1*a2 < b1*b2) ? a1*a2 : b1*b2;

	ddouble a_prime, b_prime, c_prime;
	ddouble ay_prime, by_prime, cy_prime;

	//	cout << "In find_intersections, the inputs are: " << a << b << star << endl; cout.flush ();

	a_prime = -b1*c2 + a2*b1 + a1*a2 - a2*c1;
	b_prime = a2 * (a2*b2 + c2*c2 - b2*c2 + c1*c1 - a2*c2 - a1*c1 + a1*b1 - b1*c1);
	c_prime = a2*a2 * (-b2*c1 + a1*b2 + a1*a2 - a1*c2);

	ay_prime = -b2*c1 + a1*b2 + a2*a1 - a1*c2;
	by_prime = a1 * (a1*b1 + c1*c1 - b1*c1 + c2*c2 - a1*c1 - a2*c2 + a2*b2 - b2*c2);
	cy_prime = a1*a1 * (-b1*c2 + a2*b1 + a2*a1 - a2*c1);

	discriminant = b_prime*b_prime - 4*a_prime*c_prime;
	discriminanty = by_prime*by_prime - 4*ay_prime*cy_prime;

#ifdef DEBUG_FI
	cout << "Discriminant, a, b, star = " << discriminant << a << b << star;
#endif

	intersection_pts.assign (a, 1);
	intersection_pts.assign (b, 2);

	lista_pts.assign (a, 1);
	lista_pts.assign (b, 2);

	if (discriminant < 0.0) {
		order_points (intersection_pts);
		order_points (lista_pts);
#ifdef DEBUG_FI
		cout << "Returning 2 intersections" << endl; cout.flush ();
#endif
		return (2);
	} else if (discriminant >= MEDUSA_INFINITY) {
		order_points (intersection_pts);
		order_points (lista_pts);
#ifdef DEBUG_FI
		cout << "discriminant >= MEDUSA_INFINITY" << endl; cout.flush ();
#endif
		return (2);
	} else if (discriminant > 0.0) {
		x = 1 / (2*a_prime) * (-b_prime + sqrt (discriminant));
		y1 = 1 / (2*ay_prime) * (-by_prime + sqrt (discriminanty));
		y2 = 1 / (2*ay_prime) * (-by_prime - sqrt (discriminanty));
		if (abs (hyp_const - x*y1) > abs (hyp_const - x*y2)) {
			y = y2;
			y2 = y1;
			y1 = y;
		}
		if (abs (x) > abs (y))
			root = CComplex (x, hyp_const/x);
		else
			root = CComplex (hyp_const/y, y);
#ifdef DEBUG_FI
		cout << "4 intersections: a, b, star, root = " << a << b << star << root;
#endif
		if (abs (arg (a/root)) > M_PI / 2 || abs (arg (b/root)) > M_PI / 2) {
			order_points (intersection_pts);
			order_points (lista_pts);
#ifdef DEBUG_FI
			cout << "2 out, 2 in" << endl; cout.flush ();
#endif
			return (2);
		}

		/* If the new root is actually equal to a or b, i.e.,
		 * there is are two tangencies, must return only two
		 * intersections, of course. But put in the multiple
		 * roots in lista_pts.
		 */
		if (numerical_equal (root, a) || numerical_equal (root, b)) {
			order_points (intersection_pts);
			lista_pts.assign (a, 3);
			lista_pts.assign (b, 4);
			order_points (lista_pts);
#ifdef DEBUG_FI
			cout << "2 tangencies" << endl; cout.flush ();
#endif
			return (2);
		}

		intersection_pts.assign (root, 3);
		lista_pts.assign (root, 3);

		x = 1 / (2*a_prime) * (-b_prime - sqrt (discriminant));
		if (abs (x) > abs (y2))
			root = CComplex (x, hyp_const/x);
		else
			root = CComplex (hyp_const/y, y);
		intersection_pts.assign (root, 4);
		lista_pts.assign (root, 4);

		order_points (intersection_pts);
		order_points (lista_pts);
#ifdef DEBUG_FI
		cout << "4 real intersections" << endl; cout.flush ();
#endif
		return (4);
	} else if (discriminant == 0.0) {
		x = 1 / (2*a_prime) * (-b_prime);
		root = CComplex (x, hyp_const / x); 

		if (abs (arg (a/root)) > M_PI / 2 && abs (arg (b/root)) > M_PI) {
			order_points (intersection_pts);
			order_points (lista_pts);
//			cout << "In find_intersection, ab is = " << intersection_pts [2] << endl; cout.flush ();
			return (2);
		}

		intersection_pts.assign (root, 3);
		lista_pts.assign (root, 3);
		lista_pts.assign (root, 4);

		order_points (intersection_pts);
		order_points (lista_pts);
		return (3);
	}

	order_points (intersection_pts);
	order_points (lista_pts);
	return (2);
}
/* End of find_intersections */



/* 
 * is_homotopic --
 *
 * Returns 1 iff ALL distinguished points except
 * for a, b, and STAR are outside the BAD region.
 */
int	is_homotopic (CComplex a, CComplex b, CComplex star,
		      DynArray<CComplex>& distinguished_pts,
		      CComplex& new_point)
{
	int point_i;                                   /* Index for distinguished points */
	int i_i;                                       /* Index for intersection points */
	int length = distinguished_pts.maxassigned ();
	DynArray<CComplex> intersection_pts (0);        /* Intersection points ... */
	DynArray<CComplex> lista_pts (0);               /* ... with multiplicity   */
	int num_intersections;
	CComplex rotation;
	CComplex center;
	CComplex average;
	ddouble radius;
	CComplex z;          /* Current distinguished point       */
	CComplex a_prime;    /* Temporary storage of adjacent     */
	CComplex b_prime;    /* intersection points.              */
	ddouble up_or_down;  /* Imaginary part of mobiused center */


	//cout << "is_homotopic inputs: a, b, star = " << a << b << star << endl; cout.flush ();
	
	if (abs (b) > EPSILON)
	if (abs (arg (a/b)) > M_PI/2
	    && (abs (real (b)) > EPSILON || abs (imag (b)) > EPSILON)) {
		//cout << "is_homotopic inputs: a, b, star = " << a << b << star << endl; cout.flush ();
		throw Corruption ();
	}
	
	if (numerical_equal (a, b))
		return 1;

	rotation = asymptote_rotate_angle (a, b);

	//cout << "Rotation vector = " << rotation << endl; cout.flush ();

	a *= rotation;
	b *= rotation;
	star *= rotation;
	
#ifdef DEBUG_SUB	
	cout << "Rotated inputs: a, b, star = " << a << b << star << endl; cout.flush ();
#endif

	intersection_pts.assign (a, 1);
	intersection_pts.assign (b, 2);
	order_points (intersection_pts);
	a = intersection_pts [1];
	b = intersection_pts [2];
	
	find_circle (a, b, star, center, radius, average);
	up_or_down = imag (mobius_to_line (average, a, b, star));
	/*
	if ((abs (real (a)) < EPSILON && abs (real (b)) < EPSILON) 
	    || (abs (imag (a)) < EPSILON && abs (imag (b)) < EPSILON)
	    || (abs (real (b)) < EPSILON && abs (imag (b)) < EPSILON)
	    || (abs (real (a)) < EPSILON && abs (imag (a)) < EPSILON)) {
	    */
	if (abs (real (a) * imag (a)) <= EPSILON || abs (real (b) * imag (b)) <= EPSILON) {
		if (!(abs (center) < MEDUSA_INFINITY))
			if (!(abs (star) < MEDUSA_INFINITY))
				return (1);
				/*
			    || imag ((star-a)/(a*CComplex (0,1))) * imag ((b-a)/(a*CComplex (0,1))) < 0
			    || imag ((star-b)/(b*CComplex (0,1))) * imag ((a-b)/(b*CComplex (0,1))) < 0)
				return (1);
				
			else for (point_i = 1; point_i <= length; point_i++) {
				z = distinguished_pts [point_i];
				z *= rotation;
				if (!numerical_equal (z, a) && !numerical_equal (z, b))
					if (imag ((z-a)/(b-a))*imag ((CComplex (-1, -1)-a)/(b-a)) > 0) {
						new_point = (a + b)/2;
						new_point /= rotation;
						return (0);
					}
			}
			*/
		for (point_i = 1; point_i <= length; point_i++) {
			z = distinguished_pts [point_i];
			z *= rotation;
			
			if (!numerical_equal (z, a) && !numerical_equal (z, b))
				if (lies_in_circle (z, a, b, star, up_or_down) 
				    && (imag ((z-a)/(b-a)) * imag ((star-a)/(b-a)) < 0)) {
#ifdef DEBUG_FP
					cout << "Failure pt. Circle" << endl; cout.flush ();
#endif
					//cout << "a-b = " << a-b << "a = " << a << "b = " << b << endl; cout.flush ();
					new_point = (a + b)/2;
					new_point /= rotation;
					return (0);
				}
		}
	return (1);
	}

	if (!(abs (center) < MEDUSA_INFINITY)) {
#ifdef DEBUG_FP
		cout << "Uh oh" << center << a << b << star << endl; cout.flush ();
#endif
		if (!(abs (star) < MEDUSA_INFINITY)
		    || (imag ((star-a)/((b-a)*CComplex (0,1))) * imag ((b-a)/((b-a)*CComplex (0,1))) < 0
			|| imag ((star-b)/((b-a)*CComplex (0,1))) * imag ((a-b)/((b-a)*CComplex (0,1))) < 0)) {
			for (point_i = 1; point_i <= length; point_i++) {
				z = distinguished_pts [point_i];
				z *= rotation;

				if (!numerical_equal (z, a) && !numerical_equal (z, b))
					if (lies_in_hyperbola (a, b, z) 
					    && imag ((z-a)/(b-a)) * imag ((0-a)/(b-a)) >= 0) {
						new_point = get_midpoint (a, b);
						new_point /= rotation;
						return (0);
					}
			}
		} else for (point_i = 1; point_i <= length; point_i++) {
			z = distinguished_pts [point_i];
			z *= rotation;

			if (!numerical_equal (z, a) && !numerical_equal (z, b))
				if (not_lies_in_hyperbola (a, b, z)
				    && imag ((z-a)/(b-a)) * imag ((0-a)/(b-a)) >= 0) {
					new_point = get_midpoint (a, b);
					new_point /= rotation;
					return (0);
				}
		}
	} else {
//		cout << "# intersection pts: " << find_intersections (a, b, star, intersection_pts) << endl; cout.flush ();
		/*
		if (!(abs (star) < MEDUSA_INFINITY)) {
			for (point_i = 1; point_i <= length; point_i++) {
				z = distinguished_pts [point_i];
				z *= rotation;

				if (!numerical_equal (z, a) && !numerical_equal (z, b))
					if (lies_in_hyperbola (a, b, z)
					    && (imag ((z-a)/(b-a)) * imag ((0-a)/(b-a)) >= 0)) {
#ifdef DEBUG_FP
						cout << "Failure pt. Circle is Line" << endl; cout.flush ();
#endif								    
						new_point = get_midpoint (a, b);
						new_point /= rotation;
						return (0);
					}
			}
		}
		*/
		num_intersections = find_intersections (a, b, star, intersection_pts, lista_pts);
		if (num_intersections == 2) {
			for (point_i = 1; point_i <= length; point_i++) {
				z = distinguished_pts [point_i];
				z *= rotation;
				
				if (!numerical_equal (z, a) && !numerical_equal (z, b))
					switch (lies_in_sector (b, a, center, star)) {
					case 1:
#ifdef DEBUG_SUB
						cout << "In sector BXA." << endl; cout.flush ();
#endif
						
						if (bulge_or_dip (a, b, lista_pts)) {
							if (not_lies_in_circle (z, a, b, star, up_or_down)
							    && lies_in_hyperbola (a, b, z)
							    && lies_in_sector (a, b, center, z)) {
#ifdef DEBUG_FP
								cout << "Failure pt. A" << endl; cout.flush ();
								cout << "z, a, b, star = " <<z<<a<<b<<star<<endl; cout.flush ();
#endif							
								new_point = get_midpoint (a, b);
								new_point /= rotation;
								return (0);
							}
						} else if (lies_in_circle (z, a, b, star, up_or_down) 
							   && not_lies_in_hyperbola (a, b, z)) {
#ifdef DEBUG_FP						
							cout << "Failure pt. B" << endl; cout.flush ();
							cout << "z, a, b, star = " <<z<<a<<b<<star<<endl; cout.flush ();
#endif						
							new_point = get_midpoint (a, b); 
							new_point /= rotation;
							return (0);
						}
						break;
					default:
#ifdef DEBUG_SUB
						cout << "Not in sector BXA." << endl; cout.flush ();
#endif
						if (bulge_or_dip (a, b, lista_pts)) {
							if (not_lies_in_circle (z, a, b, star, up_or_down)
							    && (not_lies_in_hyperbola (a, b, z) 
								|| not_lies_in_sector (a, b, center, z))) {
#ifdef DEBUG_FP							
								cout << "Failure pt. C" << endl; cout.flush ();
								cout << "center, radius = " << center<<radius<<endl; cout.flush ();
								cout << "z, a, b, star = " <<z<<a<<b<<star<<endl; cout.flush ();
#endif							
								new_point = get_midpoint (a, b);
								new_point /= rotation;
								return (0);
							}
						} else if (lies_in_circle (z, a, b, star, up_or_down)
							   && lies_in_hyperbola (a, b, z)) {
#ifdef DEBUG_FP						
							cout << "Failure pt. D" << endl; cout.flush ();
							cout << "z, a, b, star = " <<z<<a<<b<<star<<endl; cout.flush ();

#endif						
							new_point = get_midpoint (a, b);
							new_point /= rotation;
							return (0);
						}
					}
			}
		} else {
			//cout << "Gone thru: 4 intersection pts || "; cout.flush ();

			/* Find a in intersection_pts, start index there */
			for (i_i = 1; a != intersection_pts [i_i]; i_i++) 
				;
			while (b != intersection_pts [i_i]) {
				a_prime = intersection_pts [i_i];
				b_prime = intersection_pts [i_i + 1];
				
				for (point_i = 1; point_i <= length; point_i++) {
					z = distinguished_pts [point_i];
					z *= rotation;
				
					if (!numerical_equal (z, a_prime) || !numerical_equal (z, b_prime))
						switch (lies_in_sector (b_prime, a_prime, center, star)) {
						case 1:
#ifdef DEBUG_SUB
							cout << "In sector BXA." << endl; cout.flush ();
#endif
							if (bulge_or_dip (a_prime, b_prime, lista_pts)) {
								if (not_lies_in_circle (z, a, b, star, up_or_down) 
								    && lies_in_hyperbola (a_prime, b_prime, z)
								    && lies_in_sector (a_prime, b_prime, center,z)) {
#ifdef DEBUG_FP
									cout << "Failure pt. A34" << endl; cout.flush ();
									cout << "z, a_prime, b_prime, star = " <<z<<a_prime<<b_prime<<star<<endl; cout.flush ();
#endif
									new_point = get_midpoint (a_prime, b_prime);
									new_point /= rotation;
									return (0);
								}
							} else if (lies_in_circle (z, a, b, star, up_or_down) 
								   && not_lies_in_hyperbola (a_prime, b_prime, z)
								   && lies_in_sector (a_prime, b_prime, center, z)) {
#ifdef DEBUG_FP
								cout << "Failure pt. B34" << endl; cout.flush ();
								cout << "z, a_prime, b_prime, star = " <<z<<a_prime<<b_prime<<star<<endl; cout.flush ();
#endif
								new_point = get_midpoint (a_prime, b_prime); 
								new_point /= rotation;
								return (0);
							}
							break;
						default:
#ifdef DEBUG_SUB
							cout << "Not in sector BXA." << endl; cout.flush ();
#endif
							if (bulge_or_dip (a_prime, b_prime, lista_pts)) {
								if (not_lies_in_circle (z, a, b, star, up_or_down)
								    && (not_lies_in_hyperbola (a_prime, b_prime, z) 
									|| not_lies_in_sector (a_prime, b_prime, center, z))) {
#ifdef DEBUG_FP
									cout << "Failure pt. C34" << endl; cout.flush ();
									cout << "z, a_prime, b_prime, star = " <<z<<a_prime<<b_prime<<star<<endl; cout.flush ();
#endif
									new_point = get_midpoint (a_prime, b_prime);
									new_point /= rotation;
									return (0);
								}
							} else if ((lies_in_circle (z, a, b, star, up_or_down)
								    && lies_in_hyperbola (a_prime, b_prime, z))
								   || (lies_in_circle (z, a, b, star, up_or_down) 
								       && not_lies_in_sector (a_prime, b_prime, center, z))) {
#ifdef DEBUG_FP
								cout << "Failure pt. D34" << endl; cout.flush ();
								cout << "z, a_prime, b_prime, star = " <<z<<a_prime<<b_prime<<star<<endl; cout.flush ();
#endif
								new_point = get_midpoint (a_prime, b_prime);
								new_point /= rotation;
								return (0);
							}
						} /* switch (lies_in_sector) */
				} /* for every distinguished point */
				
				i_i++;
				
			} /* while */
		} /* else more than 2 intersections */
	} /* else normal algorithm */
	//cout << "It's homotopic!" << endl; cout.flush ();
	return (1);
}
/* End of is_homotopic */

//----------------------------------------------------------------------




int sign(ddouble x)
{
	if (x > 0)
		return 1;
	if (x < 0)
		return -1;
	return 0;
}




/*
 * can_prune --
 *
 * Sees if the circle segment through a and b
 * and star can replace the two segments 
 * through a and b, and b and c.
 */
int	can_prune (CComplex a, CComplex b, CComplex c, CComplex star,
		   DynArray<CComplex>& distinguished_pts)
{
	CComplex a_mob, b_mob, c_mob;
	CComplex z, z_mob;
	int i;
	int length = distinguished_pts.maxassigned ();

	if (numerical_equal (a, c) || numerical_equal (b, c))
		return (1);

	int star_at_inf = (norm (star) >= MEDUSA_INFINITY); 
	if (!star_at_inf)
		{
			a_mob = 1/(a - star);	   /* Mobius transformation   */
			b_mob = 1/(b - star);	   /* taking star to infinity */
			c_mob = 1/(c - star);
		}
	else
		{
			a_mob = a;
			b_mob = b;
			c_mob = c;
		}
	for (i = 1; i <= length; i++) {
		z = distinguished_pts [i];
		if (!numerical_equal (z,a) && !numerical_equal (z,b) &&
		    !numerical_equal (z,c) && !numerical_equal (z,star))
			{
				z_mob = star_at_inf ? z : 1/(z - star);
				
				if (imag ((z_mob-a_mob)/(b_mob-a_mob)) *
				    sign (imag ((c_mob-a_mob)/(b_mob-a_mob))) > -1E-3)
					
					if (imag ((z_mob-b_mob)/(c_mob-b_mob)) *
					    sign (imag ((a_mob-b_mob)/(c_mob-b_mob))) > -1E-3)
						
						if (imag ((z_mob-c_mob)/(a_mob-c_mob)) *
						    sign (imag ((b_mob-c_mob)/(a_mob-c_mob))) > -1E-3)
							{
#ifdef DEBUGGING
								cout << "Pruning failed with a=" << a_mob <<
									" b=" << b_mob << " c=" << c_mob 
								     << " z=" << z << endl; cout.flush ();
#endif
								     return (0);
							}
			}
	}
	return (1);
}
/* End of can_prune */

