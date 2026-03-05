
#if !defined(__PSOOP_H)
#define __PSOOP_H

//#define ILL_COOR  // controlling the raising of exception

#include <malloc.h>	 // Memory allocation
#include <math.h>		 // math functions
#include <iomanip.h>	 // Used to manipulate the output of ostream
#include <iostream.h> // stream output to screen
#include <fstream.h>	 // stream output to file
#include <string.h>	 // string manipulation
#include "cktypes.h"	 // The types used in the code

#define	FALSE			0
#define	TRUE			1
#define	alignsize	8					// memory alignment used normally to 8 bit
												// >>Illegal_coordinate()<< used in
												// Greybitmap::Getpixel,Greybitmap::Putpixel
#define	backgroundcolor 0x00			//The colorbitmap is initialised to(0,0,0,0)
#define	backgroundtone  0xFF			//The greybitmap is initialised to white


typedef unsigned char		Ushort;	// using char insteed of short for
												// compability with malloc .
typedef unsigned int 		Uint;
typedef unsigned long int	Ulong;

typedef unsigned char 		Pixeltype;


/*------------------------------------------------------------------------------
 * Outputtype		The input data struct for the 'ToPS()' function.
 *						The struct contains important information about the bitmap
 *						and the file to be created.
 *
 * ELEMENTS:
 *		dpi			(Uint) Dots pr. inch in the output file in PostScipt format.
 *		Filename    (char[36]) The name of the outputfile.
 *	LOG:				06.08.95 Documentation written for this native type.
 *----------------------------------------------------------------------------*/


struct	Outputtype
{
	Uint dpi;
	char Filename[36];
};


/*-----------------------------------------------------------------------------
 * Bitmap						Creates a new bitmap (either greyshade or color)
 *									in the memory. Used as the father to Greybitmap and
 *									Colorbitmap
 *
 *	 INPUT: 						NONE !!!
 *	OUTPUT:						NONE !!!
 *	LOCALS:	Bitmapkind		<prot>(Uint) The kind of the bitmap - see the emum
 *                         bmtype.
 *			   width 			<prot>(Uint) The width which the bitmap was
 *									initialised with.
 *				length			<prot>(Uint) The length which the bitmap was
 *									initialised with.
 *				array				<prot>(Ushort*) Pointer to the bitmap, when using EMS
 *									this pointer is not used.
 *          ferrtype			<prot>(enum) For future use, contains values for file
 *									errors.
 *
 *	FUNCTIONS: ~Bitmap		(virtual)() The virtual destructor for the bitmaps.
 *                         See ~Greybitmap & ~Colorbitmap for full documentation
 *				  Putpixel		(pure virtual void)(Uint x, Uint y, mixcolour color)
 *									Stores a pixel in the bitmap-see Greybitmap::Putpixel
 *									and Colorbitmap::Putpixel for full documentation.
 *            Getpixel		(pure virtual mixcolour)(Uint x, Uint y)
 *									Raeds a pixel in the bitmap-see Greybitmap::Getpixel
 *									and Colorbitmap::Getpixel for full documentation.
 *				  ToPS			(pure virtual void) (Outputtype Outputinfo,
 *									Ushort compression).Dumps the bitmap to a PostScript
 *									file - see Greybitmap::ToPS and Colorbitmap::ToPS for
 *									full documentation.
 *				 type				(pure virtual bmtype) (). Returns the type of the
 *									bitmap (colour / Grey).
 *           Fileerror		Exception class - can be used if error is incounted
 *									while working with a file - Not used yet!!!
 *				 Memory			Exception class - is thrown when a bitmap bigger than
 *									the memory size is requested.
 *           Illegal_coordinate
 *									Exception class -is thrown when an illegal coordinate
 *									is requested. Can be disabled when #define ILL_COOR
 *									is not defined.
 *				Illegal_color	Exception class -is thrown when an illegal color is
 *									requested.
 *	LOG  :						06.07.95	Class created - Only Greybitmap class exists
 *									at this time. NO support for fileerrors.
 *									06.07.95 Class expanded - now also Colorbitmap exists
 *									Thinging about how to cope with getpixel in Bitmap.
 *									This should be the same function to both color and
 *									grey (template?)
 *---------------------------------------------------------------------------*/
class	Bitmap
{
	protected :
		enum 		   ferrtype {criticalerror}; // The fileerror number
		Uint			width;						  // x-range of picture
		Uint			length;          			  // y-range of picture
		Ushort 		*array;						  // a pointer to the bitmap

	public :
		virtual ~Bitmap() {};
		virtual void Putpixel(Uint x, Uint y, mixcolour color) = 0;
		virtual mixcolour Getpixel (Uint x, Uint y)=0;
		virtual void  ToPS(Outputtype Outputinfo, Ushort compression) = 0;
		virtual bmtype	type(void) = 0;
				// here the children returns their kind of bitmap
		class    Fileerror
					{
					public :
						Ushort ferror;
						Fileerror(Ushort ferrno) {ferror = ferrno;} ;
					};

		class		Memory {};				// exception class for shortness af memory
		class		Illegal_coordinate{};// exception for illegal adressing of bitmap
		class		Illegal_color{};		// exception for illegal color value

};



/*-----------------------------------------------------------------------------
 * Greybitmap(x,y,bitplanes)	Creates a new greybitmap of size (x*y*bitplanes)/8
 *									bytes in the memory.
 *
 *									Use an EMS or XMS manager when to create a bigger
 *									bitmap that the size stated above.
 *									If the requested bitmap is bigger that the maximum
 *									free memory, an exception will be thrown, orelse the
 *									the bitmap is created and is set to 'backgroundcolor'
 *
 *	 INPUT: 	x					(*Uint) specifies the width  of the picture
 *          y					(Uint) specifies the length of the picture
 *				bitplanes   	(unsigned short int) the numbers of grayshades in the
 *									picture are given by 2^bitplanes.
 *									Legal values are 1,2,4 and 8, giving a maximum of
 *									256 grayshades. Other values will throw an exception
 *
 *	OUTPUT:						An object containing the bitmap.
 *	LOCALS: noofbitplanes	<prot>(Ushort) Number of bitplanes
 *			  EMSHandles      <priv> (int) The number of the EMShandle used.
 *
 *	FUNCTIONS:					For documentation pleace refeer to the .cpp file.
 *	INHERITANCE:				Derived from class Bitmap
 *	LOG  :						05.18.95	Class created - no support for bitmaps over
 *									64 Kbyte yet.
 *									06.01.95 Classes for exceptions Illegal_coordinate()
 *									and Illegal_color() created.
 *									06.05.95	The constructor is changed to use EMS memory
 *									this will allow bitmaps greater that 64 Kb (or 640Kb)
 *									under DOS and Windows 3.11. When using EMS *array
 *									works as a flag(improvements -storing the handler)
 *									The amound of memory has to be aligned to 32 bit so
 *									(width *length * noofbitplanes) % 32 = 0.
 *                         06.07.95 Now support for mutiple EMS blocks.
 *---------------------------------------------------------------------------*/
class Greybitmap : public Bitmap
{
  protected :
    Ushort noofbitplanes; // The number af bitplanes
	
  public :
    Greybitmap (Uint x, Uint y, Ushort bitplanes);
    ~Greybitmap ();       // destructor for the bitmap
    mixcolour Getpixel (Uint x, Uint y);
    // Get the pixel value at (x,y)
    void Putpixel (Uint x, Uint y, mixcolour color);
	// stores a pixel with 'color' at (x,y)
    void ToPS(Outputtype Outputinfo, Ushort compression);
        // Creates a POSTSCRIPT file of the picture
    bmtype type(void) {return graytone;}; // Reports the type of the bitmap
};


/*-----------------------------------------------------------------------------
 * Colorbitmap(x,y)			Creates a new Colorbitmap of size (x*y*4)bytes in the
 *									memory.
 *									This gives a total of 256^3 colors + 256 greyshades.
 *									Use an EMS or XMS manager to create bigger bitmap
 *								   that the 64 Kb.
 *									If the requested bitmap is bigger that the maximum
 *									free memory, an exception will be thrown, orelse the
 *									the bitmap is created and is set to 'backgroundCMYK'
 *
 *	 INPUT: 	x					(*Uint) specifies the width  of the picture
 *          y					(Uint) specifies the length of the picture
 *	OUTPUT:						An object containing the bitmap.
 * LOCALS:  backgroundCMYK <priv>(cmykcolour) The backgroundcolor.
 *          EMSHandles     <priv>(int) The number of the EMShandle used.
 *
 *	FUNCTIONS:					For documentation pleace refeer to the .cpp file.
 *	INHERITANCE:				Derived from class Bitmap
 *	LOG  :						06.07.95	Class created.When using EMS *array
 *									works as a flag(improvements -storing the handler)
 *									The amound of memory has to be aligned to 32 bit so
 *									(width *length * 4) % 32 = 0.
 *---------------------------------------------------------------------------*/
class Colorbitmap : public Bitmap
{
	private :
		cmykcolour backgroundCMYK;
	public :
					Colorbitmap (Uint& x, Uint y);
												// constructor with default values
					~Colorbitmap ();     // destructor for the bitmap
		mixcolour Getpixel (Uint x, Uint y);
												// Get the pixel value at (x,y)
		void		Putpixel (Uint x, Uint y, mixcolour color);
												// stores a pixel with 'color' at (x,y)
		void 		ToPS(Outputtype Outputinfo, Ushort compression);
												// Creates a POSTSCRIPT file of the picture
		bmtype type(void) {return cmyk;}; // reports type of bitmap
};

#endif // __CKPSOOP_H






