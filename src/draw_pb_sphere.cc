#include<complex.h>
#include<iostream.h>
#include"cktypes.cc"
#include"psoop.cc"
#include"dynarray.C"
#include"CP1.h"

typedef complex<double> Complex;


Uint xr;
Uint yr;
int dpi;
const Ushort bitpl = 2; // must be of the form 2^n, n in {0,1,2,3}


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
  cout << "resolution (dpi) "; cin >> dpi;
  x_min = y_min = -1;
  x_max = y_max = 1;
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



int ortho_proj_to_sphere(Complex z, CP_1& a, CP_1& b)
  // Projects the unit disk orthogonally to the sphere,
  // with respect to homogenous coordinates (on the sphere).
  // The sphere is placed such that
  // 0 projects to 1 (-1), i projects to infinity and
  // -1 to 0.
  // Return 1 if succesful, i.e. if z lies in the unit-disk
  // The results of the projection is returned in a (front point)
  // and b (hidden point)
  
{
  double r = norm(z);
  if (r > 1) return 0;
  
  a = CP_1( Complex(sqrt(1 - r), real(z)), 1 - imag(z) );
  b = CP_1( Complex(-sqrt(1 - r), real(z)), 1 - imag(z) );

  return 1;
} // ortho_proj_to_sphere



int composition_in_D(CP_1 w)
  // Returns 1 iff the composition
  // f_1 \circ f_2 \circ ... \circ f_n (w)
  // lies in the unit disk, where 
  //  f_j : z -> (a_j z^2 - a + 1) / (b_j z^2 -b + 1)
{
  Complex u, v, a, b;
  int j;
  // for each mapping
  for (j = a_params->maxassigned(); j >= 1; j-- )
    {
      a = (*a_params)[j];
      b = (*b_params)[j];
      
      u = pow(numer(w), 2);
      v = pow(denom(w), 2);
      w = CP_1( a*(u - v) + v, b*(u - v) + v );
      if (j/4*4 == j)
	w = balance(w);
    } // for
  return ( norm(numer(w)) < norm(denom(w)) );
} // composition_in_D




void draw_pullback(void)
  // draw the pullback of upper halfplane under
  // f_1 \circ f_2 \circ ... \circ f_n
  // where 
  //  f_j : z -> (a_j z^2 - a + 1) / (b_j z^2 -b + 1)

{
  Greybitmap bm(xr, yr, bitpl);
  mixcolour tone;
  Uint x, y;
  Complex a, b, z;
  CP_1 w_front, w_hidden;
  int xd = (xr + 79)/80;
  int yd = (yr + 44)/45;

  // For each pixel :
  for (y = 0; y < yr; y++) {
    for (x = 0; x < xr; x++) {
      z =  Complex(double(x_min)+(double(x_max-x_min))*x/xr, 
		   double(y_min)+(double(y_max-y_min))*y/yr);

      if (ortho_proj_to_sphere(z, w_front, w_hidden))
	if (composition_in_D(w_front))
	  tone.settone(0);
	else if (composition_in_D(w_hidden))
	  tone.settone(2);
	else
	  tone.settone(3);
      else
	tone.settone(3); 
      
      bm.Putpixel(x, yr-y-1, tone);

      // screen output
      if (x/xd*xd == x && y/yd*yd == y) { 
	if (tone.reporttone() == 3) cout << ".";
	else if (tone.reporttone() == 2) cout << ":";
	else if (tone.reporttone() == 1) cout << "+";
	else cout << "X";
	if ((xr-x) <= (xr/80)) cout << endl;
      }
    } // for
  } // for
  bm.ToPDF(ot);
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
  



