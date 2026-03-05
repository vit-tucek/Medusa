/*-----------------------------------------------------------------------------
 *  psoopems.cpp    This part of the code can create a bitmap in the memory.
 *						  Output for the bitmap can only be done in a PostScript file.
 *	  				  The reson for this is that the code is primally made for UNIX
 *					  and the use of creating pictures which is to be printed by a
 *					  PostScript laserprinter or a typesetter(for prof. use).
 *					     If the compile variable 'EXISTS_DOS' is defined the compiler
 *					  will produce a code that uses EMS memory. The choice of EMS as
 *					  memory expander on the PC under DOS was because the adressing
 *					  is faster(I tested it, it's about 40 % faster than XMS).
 *					  The code emsmem.h is taken from ftp.borland.com and the only
 *					  changes made to it, was to disable the text output, which was
 *					  produced while reading and writting in the memory.
 *					  The code emsmem.h isn't 100 % working, but this is not my fault
 *					  - I didn't make it, Borland did. The problem arrises when
 *					  trying to use more than one EMS block at the same time, ex.
 *					  generating two bitmaps at the same time, normally the bitmaps
 *					  would get different Handle numbers, and when wanting to adress
 *					  bitmap one the EMSHandles number should be set the number
 *					  which the alloc routine itself generered under allocation one,
 *               and the same thing to do if the bitmap in questing was no two.
 *					  But the second bitmap sometimes get trashed.
 *					  This implies that only one bitmap at atime can be created, if
 *					  the first bitmap is deallocated before creating the new one all
 *					  is OK. The classes 'Greybitmap' and 'Colorbitmap' each have a
 *					  EMSHandles(int) variable, this is done for future developments.
 *					  If the code is supposed to run under UNIX or WIN95, the
 *					  'EXISTS_DOS' has to undefined, so the code generered uses the
 *					  standard malloc.h routines. Under these systems, the memory
 *					  isn't limited to 64 Kb (or when using FAR pointers 640 Kb).
 *                  There are places in the code where more OOP could have been
 *					  introduced, this is not been done because of efficiency (even
 *					  though Bjarne Stroustrup(the auther of c++ OOP) says that no
 *					  overhead is introduced when using c++ OOP instead of c - THIS
 *					  IS NOT TRUE - well not in the Borland version 4.5 it isn't).
 *					     Another funny thing is the function 'ToPS', which is virtual
 *               in class Bitmap and fully declared in Grey- and Colorbitmap.
 *					  At this time alot of the code in the two versions are the same
 *					  , but for future versions, where it's quite possible that the
 *					  code is going to be quite different, they are both fully
 *					  declared instead of making a general part and a part for each
 *					  bitmap type(color or grey).
 *					     Also in the function 'ToPS' the variable 'compression'
 *					  is not at this time fully utialised, but included for future
 *					  use. (When I find out how PostScript uses LZW coding!).
 * 				     When using the EMS version of the code in a DOS window under
 *						Microsoft Windows or OS/2 the code can use virtual EMS memory,
 *					  eg. EMS 'memory' which is present in the swap file on the
 *					  hard drive. This ofcause slows down the adressing in the EMS,
 *					  but enables the use of bitmap up to 32 MB.
 *					     In the class 'Bitmap' a class 'Fileerror' exists. This is a
 *					  error class for future use by the 'ToPS' routines.
 *
 *					     The compiler variable 'ILL_COOR' controls if, when trying to
 *					  adress a illegal coordinate, the code should throw an exception
 *					  or just continue.
 *					     'backgorundcolor' contains the value, that the bitmap is
 *					  iniatialised to -both grey and color. In the color version all
 *					  four colors pr. pixel is set to this value.
 *							Remember that when creating a greyshade bitmap the number
 *               of bitplanes has to be 1, 2, 4 or 8. Other values will corrupt
 *					  the PostScript file. There is now checking for this, because
 *					  for future use other numbers could be practical.
 *
 *               		(c)	Klavs	Riisager
 *
 *				Problems with the code? Mail me at: gc922727@gbar.dtu.dk
 *                             				  or: Klaus.Riisager.mat.dtu.dk
 *----------------------------------------------------------------------------*/

#include "psoop.h"




/*-----------------------------------------------------------------------------
 * Greybitmap::Greybitmap(x,y,bitplanes)
 *                			The constructor for the class Greybitmap. Creates a
 *									bitmap in the memory of size (x*y*bitplanes)/8 bytes.
 *									If the requested bitmap is to big for the memory an
 *									exception is thrown, or else the bitmap is created
 *									and initialised to contain 'backgroundtone'
 *									When running at a PC the size is restricted to 64
 *									Kbyte at this time, to use bigger bitmaps an EMS or
 *									XMS manager have to the used.
 *	 INPUT: 	x					(Uint&) specifies the width  of the picture
 *									Has to be a multiplum of 8(when used on a PC with EMS
 *									support it has to be 32), if not the program will
 *									add a number x (1<= x <=7)(eg. 1 & 31 )
 *									so this will become true.
 *          y					(Uint) specifies the length of the picture
 *				bitplanes   	(Ushort) the numbers of grayshades in the
 *									picture are given by 2^bitplanes.
 *									Legal values are 1,2,4 and 8, giving a maximum of
 *									256 grayshades. NO exception will be throw if illegal
 *
 *	OUTPUT:						An object containing the bitmap.
 *	LOCALS:  backgcolor		(Ulong) A copy of #define backgroundtone
 *				backcolor		(Ulong) 32 bit filling of backgcolor
 *				noofbytes		(Ulong) The number of bytes needed for the bitmap.
 *				local_bitmap   (Ushort*) A copy of the pointer to the bitmap
 *				counter			(Ulong) a counter.
 * ClASS LOCALS:           Bitmap::array(Ushort*), Greybitmap::EMSHandles(int)
 *									Bitmap::width(Uint), Bitmap::length(Uint)
 *									Greybitmap::noofbitplanes(Ushort), Bitmap::Memory()
 *									Bitmap::Bitmapkind(enum), (emsmem.h - EMSHandle)
 *	USES: malloc()				(malloc.h)
 *       EMSExists()			(emsmem.h)
 *		   EMSFunctional()   (emsmem.h)
 *		   EMSAlloc()			(emsmem.h)
 *			GetEMSPageFrame() (emsmem.h)
 *			PutVal()          (emsmem.h)
 *	LOG  :						05.18.95	constructor created - Only support for
 *									normal memory allocation (malloc) at this time.
 *									Will run normal under UNIX, but is restricted to
 *									64 Kbyte when run on a PC.
 *                         06.05.95	Constructor changed to use EMS when conpiler
 *									directive EXISTS_DOS is true - remember to align to
 *									32 bit.
 *									06.07.95	Had to change (backgroundcolor(#define)<<8)
 *									to a copy of it, because you can't shift a #define.
 *									By using a VERY dirty trick (using global addressing)
 *									I have been able to allocate mulitiple EMS blocks.
 *									This is done by storing the EMShandle number in
 *									Greybitmap::EMSHandles(int)
 *									06.18.95 The bitmap is now initialised to
 *									'backgroundtone' and not 'backgroundcolor'
 *---------------------------------------------------------------------------*/

Greybitmap::Greybitmap (Uint x , Uint y, Ushort bitplanes)
{
	x = ((x+alignsize-1)/alignsize)*alignsize;
	Ulong noofBytes= (Ulong(x)/8)*y*bitplanes;
	array = (Ushort *) malloc(noofBytes);
	// if (array==NULL) throw Memory();
	width=x;
	length=y;
	noofbitplanes = bitplanes;
	Ushort *local_bitmap;
	local_bitmap = array;
	for (Ulong counter=0;(counter<noofBytes);counter++)
	{
		*(local_bitmap)= backgroundtone;
		local_bitmap++;
	}
};

/*-----------------------------------------------------------------------------
 * Greybitmap::~Greybitmap()
 *                			The destructor for the class Greybitmap. removes a
 *									bitmap from the memory.
 *	 INPUT: 						NONE !!!
 *	OUTPUT:						NONE !!!
 *	LOCALS:						NONE !!!
 * CLASS LOCALS:				Bitmap::array(Ushort*), Greybitmap::EMSHandles(int)
 *									Bitmap::Memory(), (emsmem.h - EMSHandle)
 * USES:	 free()				(malloc.h)
 *        CleanUp()			(emsmem.h)
 *	LOG  :						5.18.95	destructor created - Only support for
 *									normal memory deallocation (free) at this time.
 *									Will run normal under UNIX, but is restricted to
 *									64 Kbyte when run on a PC.
 *									06.06.95	destructor changed - Now support for EMS too
 *---------------------------------------------------------------------------*/
Greybitmap::~Greybitmap ()
{
  // if (array==NULL) throw Memory();
	free (array);
};

/*-----------------------------------------------------------------------------
 * Greybitmap::Getpixel(x,y)
 *                         Getpixel will when given a coordinate (x,y) return
 *									the value (color/colors) of the pixel. If the
 *									coordinate is illegal(greater that the bitmap itself-
 *									0<=x<=width and 0<=y<=length) the function will throw
 *									an exception(Illegal_coordinate) and 0 as returncolor
 *									This exception can be disabled by setting compiler
 *									variable:  IL_COOR to FALSE (0).
 *
 *	Abstraction: Uint * Uint -> mixcolour
 *
 *	 INPUT: 	x					(Uint) specifies the vertical coordinate
 *									of the pixel in question.
 *          y					(Uint) specifies the horisontal coordinate
 *									of the pixel in question.
 *
 *	OUTPUT:						A value of type Pixeltype containing the pixel color
 *	LOCALS:  temp				(Pixeltype) The greyscale value to return
				answer			(mixcolour) due to required type compability in the
									object hieracy, we return a mixcolour
 *				Bitmap_add		(Ulong) floor of the address of the pixel in question
 *          local_bitmap	(Ushort*) Local copy of the pointer to the bitmap
 *	CLASS LOCALS: Uses Bitmap::width(Uint),Bitmap::length(Uint)
 * 	   Greybitmap::noofbitplanes(Ushort),
 *	   Bitmap::*Array(Ushort*), Bitmap::Illegal_coordinate()
 *      Greybitmap::EMSHandles(int), (emsmem.h - EMSHandle)
 *	USES: GetVal()          (emsmem.h)
 *	LOG  : 05.31.95-06.01.95 Getpixel created - Only support for
 *				 normal memory allocation (malloc) at this time.
 *				 Will run normal under UNIX, but is restricted to
 *				 64 Kbyte when run on a PC under DOS.
 *				 06.06.95 Getpixel changed - Now support for EMS too.
 *                               Remember that EMS is addressed by a longword.
 *---------------------------------------------------------------------------*/
mixcolour Greybitmap::Getpixel (Uint x, Uint y)
{
  Pixeltype temp = 0;
  if ((x>= width) || (y>= length)) {
    #ifdef ILL_COOR
      throw Illegal_coordinate();
  };
  #else // ILL_COOR
  }  // indirectly answer = 0
  else   // If ILL_COOR = FALSE don't warn and don't set pixel
  #endif // ILL_COOR
  {
    Ushort *local_bitmap;
    local_bitmap=array;
    local_bitmap += (((Ulong(width)*y+x)*noofbitplanes)/alignsize) ;
    temp =  ( (*local_bitmap) >> (alignsize-((x)*noofbitplanes % alignsize)
					- noofbitplanes) ) & Pixeltype((2 << (noofbitplanes-1))-1) ;
  }
  mixcolour answer;
  answer.settone(temp); // answer now corresponds to temp
  return (answer);
};



/*-----------------------------------------------------------------------------
 * Greybitmap::Putpixel(x,y,color)
 *                         Putpixel will when given a coordinate (x,y) and a
 *									color try to add this to the bitmap. All pixels will
 *									be written over. If the	coordinate is illegal
 *									(greater that the bitmap itself - 0<=x<=width
 *									and 0<=y<=length) the function will throw
 *									an exception( Illegal_coordinate() ).
 *									This exception can be disabled by setting compiler
 *									variable:  IL_COOR to FALSE (0).
 *									In both casesit will not be stored if it is illegal.
 *									If a coloris illegal 0<=color< 2^noofbitplanes an
 *									exception ( Illegal_color() ).
 *
 *	Abstraction: Uint * Uint * mixcolour -> _
 *
 *	 INPUT: 	x					(Uint) specifies the vertical coordinate
 *									of the pixel in question.
 *          y					(Uint) specifies the horisontal coordinate
 *									of the pixel in question.
 *				color				(mixcolour) specifies the color of the pixel.
 *
 *	OUTPUT:						NONE !!!
 *	LOCALS:	local_bitmap	(Ushort*) Local copy of the pointer to the bitmap.
 *		  		bitmap_add		(Ulong) floor of the address of the pixel in question
 *				lword				(Ulong) The unmodified pixel read as a longword
 *          shifts			(Ushort) The value to shift the read longword to get
 *									to the pixel.
 *				shifts			(Pixeltype) as above but for NO DOS version
 *	CLASS LOCALS:				Uses Bitmap::width(Uint),Bitmap::length(Uint)
 * 								Greybitmap::noofbitplanes(Ushort),
 *                         Bitmap::Array(Ushort*),Greybitmap::EMSHandles(int)
 *									(emsmem.h - EMSHandle)
 * USES: PutVal()				(emsmem.h)
 *	LOG  :						06.01.95	Putpixel created - Only support for
 *									normal memory allocation (malloc) at this time.
 *									Will run normal under UNIX, but is restricted to
 *									64 Kbyte when run on a PC under DOS.
 *									06.06.95 Putpixel changed - Now support for EMS too.
 *									06.07.95 The need to expansion of color to 32 bit &
 *									pow(2,noofbitplanes) != 2 << noofbitplanes
 *---------------------------------------------------------------------------*/
void Greybitmap::Putpixel (Uint x, Uint y, mixcolour color)
{
	if ((x>=width) || (y>=length) || (x<0) || (y<0)) {
#ifdef ILL_COOR
	throw Illegal_coordinate();};
#else // ILL_COOR
	}
	else
#endif // ILL_COOR
		{
		/* if (color.reporttone() < 0
			|| color.reporttone() >= (2 << (noofbitplanes-1)))
				throw Illegal_color(); */
		Ushort *local_bitmap;
		local_bitmap =array;
		local_bitmap += (Ulong(width)*y +x)*noofbitplanes/alignsize;
		Pixeltype shifts=(alignsize-(x*noofbitplanes%alignsize)-noofbitplanes);
		*local_bitmap &=~(Pixeltype( (2 << (noofbitplanes-1))-1) << shifts);
		*local_bitmap |=Pixeltype( color.reporttone() <<
							(alignsize-(x*noofbitplanes % alignsize)-noofbitplanes));

	}
};



/*-----------------------------------------------------------------------------
 * Greybitmap::ToPS(Outputtype, compression) Dumps the bitmap to a file *									(name given	in Input) in POSTSCRIPT format. *									compression enables different kinds of compression to *									be applied to the bitmap (no picture loss). *
 *	Abstraction: (char* * Uint) * Ushort -> _
 *
 *	 INPUT:  Outputinfo		(struct) Consists of specific information about the
 *									way the output should be generered. (See the struct
 *									for more information)
 *			   compression		(Ushort)
 *									0 = 	No compression , bitmap stored as hex-values
 *                               giving a filesize of 2*sizeof(bitmap).
 *									1 =   No compression , bitmap stored as coded ASCII85
 * 										giving a filesize of 5/4 * sizeof(bitmap)
 *									2 =   compression , bitmap stored as LZW coded
 *
 *	OUTPUT:						a POSTSCRIPT file
 *	LOCALS:	ps_lower_x 		(const Uint) *1/72 inch from the left lower corner.
 *				ps_lower_y 		(const Uint) *1/72 inch from the bottom.
 *			   text_size		(const Uint) *1/72 inch - the font size.
 *				linespace		(const Uint) *1/72 inch - the space between the lines
 *				border_width	(const Uint) *1/72 inch - the width of the border
 * 			noofbytes		(Ulong)	The number of bytes in the bitmaps.
 * 			nooflword		(Ulong)	The number of long word in the bitmaps.
 *				width				(Ulong) a 32 bit expansion of Bitmap::width
 *				length			(Ulong) a 32 bit expansion of Bitmap::length
 *				fo					(ofstream) the output stream
 *				local_bitmap	(Ushort*) A copy of the pointer to the bitmap
 *				counter			(Ulong) a counter
 *	CLASS LOCALS:				Uses Bitmap::width(Uint),Bitmap::length(Uint)
 * 								Greybitmap::noofbitplanes(Ushort)
 *									Bitmap::Array(*Ushort), Greybitmap::EMSHandles(int)
 *									(emsmem.h - EMSHandle)
 * USES:	open()				(fstream.h)
 *			close()				(fstream.h)
 *			setw()				(iomanip.h)
 *			setfill()			(iomanip.h)
 *			hex					(iomanip.h)
 *			GetVal()				(emsmem.h)
 *	LOG  :						06.01.95	ToPS created - Only support for
 *									normal memory allocation (malloc) at this time.
 *									Will run normal under UNIX, but is restricted to
 *									64 Kbyte when run on a PC under DOS.
 *									No support for compression yet(only compression = 0).
 *									06.05.95 Function changed to take a struct as input
 *									instead of inheritance of a class Input.
 *									I have to use typecasting to get type Ushort right
 *									Problem arrises in noofbitplanes and when dumping a
 *									pixel value.
 *                         Remembered problems when multipling someting with a
 *									(in this case) word, the result has to fit in the
 *									datatype of the factor with the biggest datatype ex.
 *									{Uint x = 1000; x = x *1000 / 1000; - x*1000 can't
 *									fit in a word - result unknown.
 *									Problems solved by using typeconvension in the
 *									funcktion to Ulong.(maxint*maxint can fit in a Ulong)
 *---------------------------------------------------------------------------*/
void Greybitmap::ToPS(Outputtype Outputinfo,  Ushort compresssion)
{
  const Uint  ps_lower_x = 72;
  const Uint  ps_lower_y = 72;
  const Uint  border_width= 1;
  const Uint  text_size   = 10;
  const Uint  linespace  =  2;
  Ulong width = Bitmap::width;
  Ulong length = Bitmap::length;

  ofstream fo;
  fo.open(Outputinfo.Filename);
  fo << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
  fo << "%%Title: " << Outputinfo.Filename << endl;
  fo << "%%Creators: Christian Henriksen & Klavs Riisager" << endl;
  fo << "%%BoundingBox: " << (ps_lower_x-border_width) << " " <<
		  (ps_lower_y-border_width) << " " <<
		  ((ps_lower_x)+ (72*width)/Outputinfo.dpi+2*border_width) << " " <<
		  ((ps_lower_y)+ (72*length)/Outputinfo.dpi+border_width) << endl;
  fo << "%%Pages: 1" << endl;
  fo << "%%DocumentFonts:" << endl;
  fo << "%%EndComments" << endl;
  fo << "%%EndProlog" << endl;
  fo << "%%Page: 1 1" << endl;


/*
  fo << "newpath" << endl;  // (left side sync mark)
  fo << (ps_lower_x-border_width) << " " <<
		  (ps_lower_y-border_width+72/3*length/Outputinfo.dpi) << " moveto"<<endl;
  fo << "0 " << (72/3*length/Outputinfo.dpi) << " rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "newpath" << endl; 						// (upper side sync mark)
  fo << (ps_lower_x-border_width+72/3*width/Outputinfo.dpi) << " " <<
		  (ps_lower_y+border_width+72*length/Outputinfo.dpi) <<
		  " moveto" << endl;
  fo << (72/3*width/Outputinfo.dpi) << " 0 rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "newpath" << endl; 						// (right side sync mark)
  fo << (ps_lower_x+(72*width/Outputinfo.dpi)+2*border_width) << " " <<
		  ((ps_lower_y-border_width)+(((72/3)*length)/Outputinfo.dpi)) <<
		  " moveto" << endl;
  fo << "0 " << (((72/3)*length)/Outputinfo.dpi) << " rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "newpath" << endl; 						// (lower side sync mark)
  fo << (ps_lower_x-border_width+(72/3*width/Outputinfo.dpi)) << " " <<
		  (ps_lower_y-border_width) << " moveto" << endl;
  fo << (72/3*width/Outputinfo.dpi) << " 0 rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;
*/


  fo << "/origstate save def" << endl;
  fo << "20 dict begin" << endl;
  fo << "/pix " << int((width*noofbitplanes+7)/8) << " string def" << endl;
  fo << (ps_lower_x) << " " << (ps_lower_y) << " translate" << endl;
  fo << double(72)*width/Outputinfo.dpi << " " <<
		  double(72)*length/Outputinfo.dpi <<	" scale" << endl;
  fo << width << " " << length << " " << short(noofbitplanes) << " [" <<
		  width << " 0 0 -" << length << " 0 " << length << "]" << endl;
  fo << "{currentfile pix readhexstring pop}" << endl;
  fo << "image" << endl;
  Ushort *local_bitmap=array;
  Ulong noofbytes= Ulong(width)/8*length*noofbitplanes;
  for (Ulong counter=0;(counter<noofbytes);counter++)
  {
	  fo << setw(2) << setfill('0') <<  hex << short(*local_bitmap);
												// We have to print -  ex. 0F and not F
	  if ((counter % 40)==0) fo << endl; // look out for long lines
	  local_bitmap++;
  }
  fo << endl;
  fo << "showpage" << endl;
  fo << "end" << endl;
  fo << "origstate restore" << endl;
  fo << "%%Trailer" << endl;
  fo.close();
};


/*-----------------------------------------------------------------------------
 * Colorbitmap::Colorbitmap(x,y)
 *                			The constructor for the class Colorbitmap. Creates a
 *									bitmap in the memory of size (x*y*4) bytes.
 *									If the requested bitmap is to big for the memory an
 *									exception is thrown, or else the bitmap is created
 *									and initialised to contain 'backgroundCMYK'
 *									When running at a PC the size is restricted to 64
 *									Kbyte at this time, to use bigger bitmaps an EMS or
 *									XMS manager have to the used.
 *									The constant backgroundCMYK is initialised here.
 *									The variable Bitmap::bitmapkind <prot> is initialised
 *									to the enum greytone (see 'types.h')
 *	 INPUT: 	x					(Uint&) specifies the width  of the picture
 *									Has to be a multiplum of 8(when used on a PC with EMS
 *									support it has to be 32), if not the program will
 *									add a number x (1<= x <=7)(eg. 1 & 31 )
 *									so this will become true.
 *          y					(Uint) specifies the length of the picture
 *	OUTPUT:						An object containing the bitmap.
 *	LOCALS:  backcolor		(Ulong) The backgroundCMYK tuble (C,M,Y,K)
 *				noofBytes		(Ulong)	The numbers of bytes in the bitmap
 *				local_bitmap	(Ushort*) A local copy of the pointer to the bitmap
 *				counter			(Ulong) A counter
 *	CLASS LOCALS: 				Colorbitmap::backgroundCMYK(cmykcolour),
 *                         Bitmap::array(Ushort*), Colorbitmap::EMSHandles(int)
 *									(emsmem.h - EMSHandle), Bitmap::width(Ulong),
 *									Bitmap::length(Ulong), Bitmap::Bitmapkind(enum)
 *									Bitmap::Memory()
 *	USES:    malloc()			(malloc.h)
 *				EMSExists() 	(emsmem.h)
 *				EMSFunctional()(emsmem.h)
 *				EMSAlloc()		(emsmem.h)
 *				PutVal()			(emsmem.h)
 *				GetEMSPageFrame()(emsmem.h)
 *	LOG  :						06.07.95	Constructor created. EMShandles initialised
 *---------------------------------------------------------------------------*/

Colorbitmap::Colorbitmap (Uint& x , Uint y)
{
	backgroundCMYK = setcmyk(backgroundcolor, backgroundcolor,
							backgroundcolor, backgroundcolor);
	x = ((x+alignsize-1)/alignsize)*alignsize;
	Ulong noofBytes= Ulong(x)*y*4;				// There are four colors.
	array = (Ushort *) malloc(noofBytes);
	// if (array==NULL) throw Memory();
	width=x;
	length=y;
	Ushort *local_bitmap;
	local_bitmap = array;
	for (Ulong counter=0;(counter<noofBytes);counter++)
	{
		*(local_bitmap)=backgroundcolor;
		local_bitmap++;
	}
};

/*-----------------------------------------------------------------------------
 * Colorbitmap::~Colorbitmap()
 *                			The destructor for the class Colorbitmap. removes a
 *									bitmap from the memory.
 *	 INPUT: 						NONE !!!
 *	OUTPUT:						NONE !!!
 *	LOCALS:						NONE !!!
 *	CLASS LOCALS:				Bitmap::array(Ushort*), Colorbitmap::EMSHandles(int)
 *									(emsmem.h - EMSHandle), Bitmap::Memory()
 *	USES:	   free()			(malloc.h)
 *				CleanUp()		(emsmem.h)
 *	LOG  :						06.07.95	destructor created.
 *---------------------------------------------------------------------------*/
Colorbitmap::~Colorbitmap ()
{
  // if (array==NULL) throw Memory();
	free (array);
};

/*-----------------------------------------------------------------------------
 * Colorbitmap::Getpixel(x,y)
 *                         Getpixel will when given a coordinate (x,y) return
 *									the value of the pixel in a mixcolour. If the
 *									coordinate is illegal(greater that the bitmap itself-
 *									0<=x<=width and 0<=y<=length) the function will throw
 *									an exception(Illegal_coordinate) and the
 *									backgroundcolor as returncolor.
 *									This exception can be disabled by setting compiler
 *									variable:  IL_COOR to FALSE (0).
 *
 *	Abstraction: Uint * Uint -> mixcolour
 *
 *	 INPUT: 	x					(Uint) specifies the vertical coordinate
 *									of the pixel in question.
 *          y					(Uint) specifies the horisontal coordinate
 *									of the pixel in question.
 *
 *	OUTPUT:						A value of mixcolortype containing the pixel color.
 *	LOCALS:  temp				(cmykcolour) holds the answer in cmyk format
 *			   answer			(mixcolour) The value to return
 *          lanswer			(Ulong) The longword containing the CMYK tuple.
 *				Bitmap_add		(Ulong) floor of the address of the pixel in question
 *          local_bitmap	(Ushort*) Local copy of the pointer to the bitmap
 *	CLASS LOCALS:				Uses Bitmap::width(Uint), Bitmap::length(Uint),
 *									Bitmap::Array(*Ushort), Bitmap::Illegal_coordinate()
 *									ColorBitmap::EMSHandles(int), (emsmem.h - EMSHandle)
 * USES:    GetVal()			(emsmem.h)
*	LOG  :						06.07.95 Getpixel created - Still thinking how to
 *									do with the 'father' function Getpixel (template?)
 *---------------------------------------------------------------------------*/
mixcolour Colorbitmap::Getpixel (Uint x, Uint y)
{
	cmykcolour temp = backgroundCMYK;
	if ((x>= width) || (y>= length)) {
#ifdef ILL_COOR
	throw Illegal_coordinate();};
#else // ILL_COOR
	}								// indirectly answer = backgorundCMYK
	else							// If ILL_COOR = FALSE don't warm and don't set pixel
#endif // ILL_COOR
	{
		Ushort *local_bitmap;
		local_bitmap=array;
		local_bitmap += (Ulong(width)*y+x)*4 ;
		temp.c = *local_bitmap;
		temp.m = *(local_bitmap+1);
		temp.y = *(local_bitmap+2);
		temp.k = *(local_bitmap+3);
	}
	mixcolour answer;
	answer.setcolour(temp); // answer corresponds to temp now
	return (answer);
};

/*-----------------------------------------------------------------------------
 * Colorbitmap::Putpixel(x,y,color)
 *                         Putpixel will when given a coordinate (x,y) and a
 *									color try to add this to the bitmap. All pixels will
 *									be written over. If the	coordinate is illegal
 *									(greater that the bitmap itself - 0<=x<=width
 *									and 0<=y<=length) the function will throw
 *									an exception( Illegal_coordinate() ).
 *									This exception can be disabled by setting compiler
 *									variable:  IL_COOR to FALSE (0).
 *									In both casesit will not be stored if it is illegal.
 *
 *	Abstraction: Uint * Uint * mixcolour -> _
 *
 *	 INPUT: 	x					(Uint) specifies the vertical coordinate
 *									of the pixel in question.
 *          y					(Uint) specifies the horisontal coordinate
 *									of the pixel in question.
 *				color				(mixcolour) specifies the color of the pixel.
 *
 *	OUTPUT:						NONE !!!
 *	LOCALS:	local_bitmap	(Ushort*) Local copy of the pointer to the bitmap.
 *		  		bitmap_add		(Ulong) floor of the address of the pixel in question
 *				lword				(Ulong) The unmodified pixel read as a longword
 *	CLASS LOCALS:				Uses Bitmap::width(Uint), Bitmap::length(Uint),
 *									Bitmap::*Array(*Ushort), ColorBitmap::EMSHandles(int)
 *									(emsmem.h - EMSHandle), Bitmap::Illegal_coordinate()
 * USES:    PutVal()			(emsmem.h)
 *	LOG  :						06.07.95	Putpixel created.
 *---------------------------------------------------------------------------*/
void Colorbitmap::Putpixel (Uint x, Uint y, mixcolour color)
{
	if ((x>=width) || (y>=length) || (x<0) || (y<0)) {
#ifdef ILL_COOR
	throw Illegal_coordinate();};
#else // ILL_COOR
	}
	else
#endif // ILL_COOR
		{
		Ushort *local_bitmap;
		local_bitmap =array;
		local_bitmap += (Ulong(width)*y +x)*4;
		*local_bitmap     = color.reportcolour().c;
		*(local_bitmap+1) = color.reportcolour().m;
		*(local_bitmap+2) = color.reportcolour().y;
		*(local_bitmap+3) = color.reportcolour().k;

	}
};



/*-----------------------------------------------------------------------------
 * Colorbitmap::ToPS(Outputtype, compression) Dumps the bitmap to a file *									(name given	in Input) in POSTSCRIPT format. *									compression enables different kinds of compression to *									be applied to the bitmap (no picture loss). *
 *	Abstraction: (char* * Uint) * Ushort -> _
 *
 *	 INPUT:  Outputinfo		(struct) Consists of specific information about the
 *									way the output should be generered. (See the struct
 *									for more information)
 *			   compression		(Ushort)
 *									0 = 	No compression , bitmap stored as hex-values
 *                               giving a filesize of 2*sizeof(bitmap).
 *									1 =   No compression , bitmap stored as coded ASCII85
 * 										giving a filesize of 5/4 * sizeof(bitmap)
 *									2 =   compression , bitmap stored as LZW coded
 *
 *	OUTPUT:						a POSTSCRIPT file
 *	LOCALS:	ps_lower_x 		(const Uint) *1/72 inch from the left lower corner.
 *				ps_lower_y 		(const Uint) *1/72 inch from the bottom.
 *			   text_size		(const Uint) *1/72 inch - the font size.
 *				linespace		(const Uint) *1/72 inch - the space between the lines
 *				border_width	(const Uint) *1/72 inch - the width of the border
 * 			noofbytes		(Ulong)	The number of bytes in the bitmaps.
 * 			nooflword		(Ulong)	The number of long word in the bitmaps.
 *				width				(Ulong) a 32 bit expansion of Bitmap::width
 *				length			(Ulong) a 32 bit expansion of Bitmap::length
 *				fo					(ofstream) the output stream
 *				local_bitmap	(Ushort*) A copy of the pointer to the bitmap
 *				counter			(Ulong) a counter
 *	CLASS LOCALS:				Uses Bitmap::width(Uint),Bitmap::length(Uint)
 *									Bitmap::Array(*Ushort), Colorbitmap::EMSHandles(int)
 *									(emsmem.h - EMSHandle)
 * USES:	open()				(fstream.h)
 *			close()				(fstream.h)
 *			setw()				(iomanip.h)
 *			setfill()			(iomanip.h)
 *			hex					(iomanip.h)
 *			GetVal()				(emsmem.h)
 *	LOG  :						06.07.95	ToPS created. At this time only support for
 *									'no compression' or compression=0.(all values are
 *									accepted)
 *---------------------------------------------------------------------------*/
void Colorbitmap::ToPS(Outputtype Outputinfo,  Ushort compresssion)
{
  const Uint  ps_lower_x = 72;
  const Uint  ps_lower_y = 72;
  const Uint  border_width= 1;
  const Uint  text_cpi   = 10;
  const Uint  linespace  =  2;
  Ulong width = Bitmap::width;
  Ulong length = Bitmap::length;

  ofstream fo;
  fo.open(Outputinfo.Filename);
  fo << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
  fo << "%%Title: " << Outputinfo.Filename << endl;
  fo << "%%Creators: Christian Henriksen & Klavs Riisager" << endl;
  fo << "%%BoundingBox: " << (ps_lower_x-border_width) << " " <<
		  (ps_lower_y-border_width) << " " <<
		  ((ps_lower_x)+ (72*width)/Outputinfo.dpi+2*border_width) << " " <<
		  ((ps_lower_y)+ (72*length)/Outputinfo.dpi+border_width) << endl;
  fo << "%%Pages: 1" << endl;
  fo << "%%DocumentFonts:" << endl;
  fo << "%%EndComments" << endl;
  fo << "%%EndProlog" << endl;
  fo << "%%Page: 1 1" << endl;
  fo << "newpath" << endl; 						// (left side sync mark)
  fo << (ps_lower_x-border_width) << " " <<
		  (ps_lower_y-border_width+72/3*length/Outputinfo.dpi) << " moveto"<<endl;
  fo << "0 " << (72/3*length/Outputinfo.dpi) << " rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "newpath" << endl; 						// (upper side sync mark)
  fo << (ps_lower_x-border_width+72/3*width/Outputinfo.dpi) << " " <<
		  (ps_lower_y+border_width+72*length/Outputinfo.dpi) <<
		  " moveto" << endl;
  fo << (72/3*width/Outputinfo.dpi) << " 0 rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "newpath" << endl; 						// (right side sync mark)
  fo << (ps_lower_x+(72*width/Outputinfo.dpi)+2*border_width) << " " <<
		  ((ps_lower_y-border_width)+(((72/3)*length)/Outputinfo.dpi)) <<
		  " moveto" << endl;
  fo << "0 " << (((72/3)*length)/Outputinfo.dpi) << " rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "newpath" << endl; 						// (lower side sync mark)
  fo << (ps_lower_x-border_width+(72/3*width/Outputinfo.dpi)) << " " <<
		  (ps_lower_y-border_width) << " moveto" << endl;
  fo << (72/3*width/Outputinfo.dpi) << " 0 rlineto" << endl;
  fo << "closepath" << endl;
  fo << border_width << " setlinewidth" << endl;
  fo << "stroke" << endl;

  fo << "/origstate save def" << endl;
  fo << "20 dict begin" << endl;
  fo << "/pix " << int((width*4+7)/8) << " string def" << endl;
  fo << (ps_lower_x) << " " << (ps_lower_y) << " translate" << endl;
  fo << double(72)*width/Outputinfo.dpi << " " <<
		  double(72)*length/Outputinfo.dpi <<	" scale" << endl;
  fo << width << " " << length << " 8" << " [" <<
		  width << " 0 0 -" << length << " 0 " << length << "]" << endl;
  fo << "{currentfile pix readhexstring pop}" << endl;
  fo << "false 4 colorimage" << endl;
  Ushort *local_bitmap=array;
  Ulong noofbytes= Ulong(width)*length*4;
  for (Ulong counter=0;(counter<noofbytes);counter++)
  {
	  fo << setw(2) << setfill('0') <<  hex << short(*local_bitmap);
												// We have to print -  ex. 0F and not F
	  if ((counter % 40)==0) fo << endl; // look out for long lines
	  local_bitmap++;
  }
  fo << endl;
  fo << "showpage" << endl;
  fo << "end" << endl;
  fo << "origstate restore" << endl;
  fo << "%%Trailer" << endl;
  fo.close();
};


