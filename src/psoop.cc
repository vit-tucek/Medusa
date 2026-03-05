/*-----------------------------------------------------------------------------
 * psoop.cc
 *
 * Bitmap rendering/output support for Medusa.
 * The original code targeted PostScript; the active output path is now PDF.
 *-----------------------------------------------------------------------------
 */

#include "psoop.h"

static void WritePdfImage(Outputtype Outputinfo,
                          Ulong width,
                          Ulong length,
                          const Ushort* image_data,
                          Ulong image_bytes,
                          const char* color_space,
                          Ushort bits_per_component)
{
  double dpi = (Outputinfo.dpi == 0) ? 72.0 : double(Outputinfo.dpi);
  double width_pt = 72.0 * double(width) / dpi;
  double height_pt = 72.0 * double(length) / dpi;

  char content_stream[256];
  int content_len = sprintf(content_stream,
                            "q\n%.6f 0 0 %.6f 0 %.6f cm\n/Im0 Do\nQ\n",
                            width_pt, -height_pt, height_pt);

  ofstream fo(Outputinfo.Filename, ios::binary);
  if (!fo) return;

  long offsets[7];

  fo << "%PDF-1.4\n";

  offsets[1] = long(fo.tellp());
  fo << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";

  offsets[2] = long(fo.tellp());
  fo << "2 0 obj\n<< /Type /Pages /Count 1 /Kids [3 0 R] >>\nendobj\n";

  offsets[3] = long(fo.tellp());
  fo << "3 0 obj\n<< /Type /Page /Parent 2 0 R\n";
  fo << "   /MediaBox [0 0 " << width_pt << " " << height_pt << "]\n";
  fo << "   /Resources << /XObject << /Im0 5 0 R >> >>\n";
  fo << "   /Contents 4 0 R >>\nendobj\n";

  offsets[4] = long(fo.tellp());
  fo << "4 0 obj\n<< /Length " << content_len << " >>\nstream\n";
  fo.write(content_stream, content_len);
  fo << "endstream\nendobj\n";

  offsets[5] = long(fo.tellp());
  fo << "5 0 obj\n<< /Type /XObject /Subtype /Image\n";
  fo << "   /Width " << width << " /Height " << length << "\n";
  fo << "   /ColorSpace /" << color_space << "\n";
  fo << "   /BitsPerComponent " << short(bits_per_component) << "\n";
  fo << "   /Length " << image_bytes << " >>\nstream\n";
  fo.write((const char*)image_data, image_bytes);
  fo << "\nendstream\nendobj\n";

  offsets[6] = long(fo.tellp());
  fo << "6 0 obj\n<< /Producer (Medusa PDF renderer) >>\nendobj\n";

  long xref_pos = long(fo.tellp());
  fo << "xref\n0 7\n";
  fo << "0000000000 65535 f \n";
  for (int i = 1; i <= 6; i++) {
    fo << setw(10) << setfill('0') << offsets[i] << " 00000 n \n";
  }
  fo << "trailer\n<< /Size 7 /Root 1 0 R /Info 6 0 R >>\n";
  fo << "startxref\n" << xref_pos << "\n%%EOF\n";
  fo.close();
}




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




void Greybitmap::ToPDF(Outputtype Outputinfo)
{
  Ulong width = Bitmap::width;
  Ulong length = Bitmap::length;
  Ulong noofbytes = Ulong(width)/8*length*noofbitplanes;
  WritePdfImage(Outputinfo,
                width,
                length,
                array,
                noofbytes,
                "DeviceGray",
                noofbitplanes);
}


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




void Colorbitmap::ToPDF(Outputtype Outputinfo)
{
  Ulong width = Bitmap::width;
  Ulong length = Bitmap::length;
  Ulong noofbytes = Ulong(width)*length*4;
  WritePdfImage(Outputinfo,
                width,
                length,
                array,
                noofbytes,
                "DeviceCMYK",
                8);
}
