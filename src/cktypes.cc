/*--------------------------------------------------------------------------
 * Code file for (cktypes) - Different typedefinitions for the code.
 *
 *			Christian Henriksen
 *			Klavs Riisager
 *			Michael Christensen
 *------------------------------------------------------------------------*/
#include "cktypes.h"


cmykcolour setcmyk (Byte cc, Byte mm, Byte yy, Byte kk)
{
	cmykcolour c;
	c.c = cc;
	c.m = mm;
	c.y = yy;
	c.k = kk;
	return c;
}; // cmykcolour setcmyk (Byte, Byte, Byte, Byte);

cmykcolour mixcolour::reportcolour(void)
{
  //	if (f != cmyk) throw CmykToneMismatch();
  //	else return c;
  return c;
}; // mixcolour::reportcolour(void)



Byte mixcolour::reporttone(void)
{
  //	if (f != graytone) throw CmykToneMismatch();
  //	else return tone;
  return tone;
}; // mixcolour::reporttone(void)
