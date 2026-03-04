#include "medusa_B.h"
#include <string.h>

void main(void) 
{
  
  long int p1, p2, q1, q2;
  
  cout << "Mating p1/q1 with p2/q2.\nEnter p1 : ";
  cin >> p1;
  cout << "Enter q1 : ";
  cin >> q1;
  cout << "Enter p2 : ";
  cin >> p2;
  cout << "Enter q2 : ";
  cin >> q2;

  if (cin.eof()) return 0;

  Medusa m(p1*q2, p2*q1, q1*q2);
  ofstream f, params;
  char ofname[36];
  f.open("medusa_txt");
  f << m;
  f.close();
  
  cout << "Output filename (enter d to use param.txt) : ";
  cin >> ofname;
  params.open( (strcmp(ofname, "d") == 0) ? "param.txt" : ofname);
  
  int n;
  
  cout << "Enter number of iterates or q to quit ";
  try {
    while (cin >> n)
      {
        for (int i = 0; i < n; i++)
          {
	    m.iterate();
	    cout << "a=" << m.param_a() << " b=" << m.param_b() << endl;
	    params << "/ " << m.param_a() << " " << m.param_b() << endl;
	  }
	f.open("medusa_txt");
	f << m;
	f.close();
	cout << "Enter number of iterates or q to quit ";
      }
  }
  catch (Corruption)
    {
      cout << "Sorry, iteration cannot continue" << endl;
    }

  params << '*' << endl;
  params.close();

} // main


