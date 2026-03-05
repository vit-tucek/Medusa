// #include<math.h>
#include<complex.h>
#include<iostream.h>
#include"cktypes.cc"
#include"psoop.cc"
#include"dynarray.C"

typedef complex<double> Complex;


Uint xr;
Uint yr;
int dpi;
const Ushort bitpl = 1; // must be of the form 2^n, n in {0,1,2,3}


typedef DynArray<Complex> Param_list;
typedef char Str36[36];

Param_list *a_params;
Param_list *b_params;

Str36 read_file_name;
double x_min, x_max, y_min, y_max;
Outputtype ot;


void getinput(void)
{
  cout << "Output filename "; cin >> ot.Filename;
  cout << "Read from file "; cin >> read_file_name;
  cout << "min (Re(z)) "; cin >> x_min;
  cout << "max (Re(z)) "; cin >> x_max;
  cout << "min (Im(z)) "; cin >> y_min;
  cout << "max (Im(z)) "; cin >> y_max;
  cout << "resolution (dpi) "; cin >> dpi;
  xr = int(5*dpi/6)*8;
  yr = 8*int ((y_max - y_min)/(x_max-x_min)*xr/8);
  cout << "produces a " << xr << " by " << yr << " bitmap" << endl; 
  ot.dpi = dpi;
} // getinput



int read_file(void)
  // read lists of a and b parameters of the form
  //   a_1 b_1
  //   a_2 b_2
  //     ...
  //   a_n b_n
  // from a file `parameters_txt'
  // stores values in *a_params and *b_params 
  // Returns 1 if the file handling is succesful, otherwise returns 0

{
  int j = 0;
  Complex parameter;
  ifstream input_file;
  
  input_file.open(read_file_name);
  
  char control_char;
  
  input_file >> control_char;

  cout << "Reading data..." << endl;
  while ( !input_file.eof() && control_char != '*')
    { 
      j++;

      // reading parameter a
      input_file >> parameter; 
      a_params->assign(parameter, j);
      cout << parameter << ", ";

      // reading parameter b
      input_file >> parameter;
      b_params->assign(parameter, j);
      cout << parameter << endl;
      
      input_file >> control_char;
    } // while

  input_file.close();
  return 1;
} // read_file



void draw_pullback(void)
  // draw the pullback of upper halfplane under
  // f_1 \circ f_2 \circ ... \circ f_n
  // where 
  //  f_j : z -> (a_j z^2 - a + 1) / (b_j z^2 -b + 1)

{
  Greybitmap bm(xr, yr, bitpl);
  mixcolour tone;
  Uint x, y;
  int j;
  Complex a, b, z, u;
  int xd = (xr + 79)/80;
  int yd = (yr + 79)/80;

  int n = a_params->maxassigned();

  // For each pixel :
  for (y = 0; y < yr; y++) {
    for (x = 0; x < xr; x++) {
      z =  Complex(double(x_min)+(double(x_max-x_min))*x/xr, 
		   double(y_min)+(double(y_max-y_min))*y/yr);
      // for each mapping
      for (j = n; j >= 1; j-- )
	{
	  a = (*a_params)[j];
	  b = (*b_params)[j];

	  u = (b*z*z - b + 1);
	  if (norm(u) > 1e-24)
	    z = (a*z*z - a + 1)/u;
	  else 
	    z = 1e12;

	} // for

      if ( norm(z) < 1 )
	tone.settone(0);
      else
	tone.settone(1);
      bm.Putpixel(x, yr-y-1, tone);

      // screen output
      if (x/xd*xd == x && y/yd*yd == y) { 
	if (tone.reporttone() == 1) cout << ".";
	else cout << "X";
	if ((xr-x) <= (xr/80)) cout << endl;
      }
    } // for
  } // for
  bm.ToPS(ot, 0);
} // draw_pullback


int main(void)
{
  // initialization
  a_params = new Param_list (0);
  b_params = new Param_list (0);
  getinput();

  // reading input and making drawing
  if ( read_file() )
    draw_pullback();
  else
    cout << "File-reading error" << endl;

  // clean up
  delete a_params;
  delete b_params;
  return 0;
} // main
  




