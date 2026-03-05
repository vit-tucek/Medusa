#include"medusa_B.h"
#include<fstream.h>
#include "dynarray.C"


const int M = 50; // max no of iterations

ofstream s; // output is directed to this stream


void estimparam(long int p1, long int p2, long int q, int max_it,
		CComplex& a, CComplex& b, int& no_it)
{

  DynArray<CComplex> a_params(CComplex(0));
  DynArray<CComplex> b_params(CComplex(0));
  Medusa *C;

  
  a_params.resetmaxassigned();
  b_params.resetmaxassigned();
  C = new Medusa(p1, p2, q);
  no_it = 1;
  int ok = 1;
  
  while (ok && no_it <= max_it)
    {
      ok = C->iterate();
      a_params.assign(C->param_a(), no_it);
      b_params.assign(C->param_b(), no_it);
      no_it++;
    }
  
  int best_entry = 1;
  double diff;
  double min_diff = 1E10;
  int k;
  for (k = 2; k <= no_it; k++)
    {
      diff = norm(a_params[k] - a_params[k-1]) + 
	norm(b_params[k] - b_params[k-1]);
      if (diff < min_diff)
	{
	  min_diff = diff;
	  best_entry = k;
	}
    }
  a = a_params[best_entry];
  b = b_params[best_entry];
  no_it = best_entry;
  
  delete C;
}
  



void fareysurvey(long int p1, long int q1, long int p2, long int q2, 
		 int depth)
{
  if (depth == 0)
    {
      CComplex a;
      CComplex b;
      int no_it;
      estimparam(p1*7, q1*3, q1*7, M, a, b, no_it);

      s << p1 << "/" << q1 << " _v_ 3/7  a = " << a << " b = "
	<< b << " #lifts " << no_it << endl;
    }
  else
    {
      fareysurvey(p1, q1, p1+p2, q1+q2, depth-1);
      fareysurvey(p1+p2, q1 + q2, p2, q2, depth-1);
    }
}
  
int main (void)
{
  s.open("mating_data.txt");

  fareysurvey(1, 7, 1, 3, 7);

  s.close();
  return 0;
}
