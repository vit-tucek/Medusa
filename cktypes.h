/*--------------------------------------------------------------------------
 * Header file for (cktypes) - Different typedefinitions for the code.
 *
 *			Christian Henriksen
 *			Klavs Riisager
 *			Michael Christensen
 *------------------------------------------------------------------------*/


#ifndef __CKTYPES_H
#define __CKTYPES_H

#include <iostream.h>
#include <complex.h>
typedef complex<double> Complex;


enum ftype {juliaset, mandelbrotset}; // The two types of math sets

enum bmtype {graytone, cmyk};         // The two types of bitmaps

typedef unsigned short Byte;

typedef long PosType;

typedef struct              			// Struct for storing start and end
{                                   // point for a plot for a ray.
	Complex startpoint;
	int from;
	int to;
} PlotRange;

class cmykcolour // The colour class 'cmykcolour'. Contains
{
  public :
		Byte c;
		Byte m;
		Byte y;
		Byte k;
	// Vi tillader fuld tilgang til c, m, y og k - der kan ikke
	// g°res megen skade da alle v†rdier (0..255) er lovlige
		int operator == (cmykcolour co) // The equal operator is defined
		{
			return (c == co.c && m == co.m && y == co.y && k == co.k);
		};
	// The piping operator << is defined, so a colour can be printed to the
	// stream in use, ex. (30, 40, 50, 60)
	friend ostream& operator << (ostream& stream, cmykcolour& c)
	{
		stream << '(' << c.c << ", " << c.m;
		stream << ", " << c.y << ", " << c.k << ") ";
		return stream;
	};
}; // cmykcolour  colortype;


cmykcolour setcmyk (Byte cc, Byte mm, Byte yy, Byte kk);


// class def. for a union of grey and colour, so a function either can
// return a colour or a greytone.
class mixcolour
{
	private :
		bmtype f;
		union
		{
			cmykcolour c;
			Byte tone;
		}; // union
	public :
		mixcolour(void) {f = graytone; tone = 0;};// constructor
		class CmykToneMismatch { }; 					// exception
		void setcolour(cmykcolour cc) {f = cmyk; c = cc;};// set a colour
		void settone (Byte t) {f = graytone; tone = t;};// set a greytone
		bmtype reporttype (void) {return f;};     // returns the type a mixcolour
		cmykcolour reportcolour(void);				// returns the colour
		Byte reporttone(void);							// returns the greytone
		int operator == (mixcolour& m)				// The equal operator is defined
		{
			return (f == graytone) ?
				(m.f == graytone && tone == m.reporttone() ) :
				(m.f == cmyk && c == m.reportcolour() );

		}; // operator == (mixcolour&)
		// The piping operator << is defined, so a mixcolour can be printed to
		// the stream in use.
	friend ostream& operator << (ostream& stream, mixcolour& m)
	{
		if (m.f == cmyk) return (stream << m.c);
		else return (stream << m.tone);
	}; // operator << (ostream&, mixcolour&)
}; // class mixcolour;


typedef struct             					 		// Contains the target for the
{																// drawing of the Mandelbrot
  char valgt;
  Complex target;
} targetvaltype;

#endif



