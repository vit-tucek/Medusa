#ifndef __CP
#define __CP

#include <complex.h>
typedef complex<double> Complex;

class CP_1 {
  
  public :
            CP_1(void);
            CP_1(const Complex&);
            CP_1(const Complex&, const Complex&);
	    CP_1(const CP_1&);
    Complex numer(void) const;
    Complex denom(void) const;
    CP_1    operator = (const CP_1&);

            operator Complex (void);
  
  protected :
    Complex u;    
    Complex v;

}; // CP_1


// function declarations

int   operator== (const CP_1&, const CP_1&);
int   operator!= (const CP_1&, const CP_1&);

CP_1 operator- (const CP_1&);
CP_1 operator+ (const CP_1&, const CP_1&);
CP_1 operator- (const CP_1&, const CP_1&);
CP_1 operator* (const CP_1&, const CP_1&);
CP_1 operator/ (const CP_1&, const CP_1&);

double norm(const CP_1&);
double abs(const CP_1&);

Complex numer(const CP_1&);
Complex denom(const CP_1&);

CP_1 balance(const CP_1&);
CP_1 invert(const CP_1&);
CP_1 conj(const CP_1&);

ostream& operator << (ostream&, const CP_1&);




// inline member definitions

inline CP_1::CP_1(void) { };

inline CP_1::CP_1(const Complex& x)
{ 
  u = x; v = 1.0; 
};

inline CP_1::CP_1(const Complex& p, const Complex& q)
{
  u = p; v = q;
};

inline CP_1::CP_1(const CP_1& x)
{
  u = x.u;
  v = x.v;
};

inline CP_1 CP_1::operator = (const CP_1& x)
{
  this->u = x.u;
  this->v = x.v;
  return (*this);
};
    
inline Complex CP_1::numer(void) const
{
  return (u);
};

inline Complex CP_1::denom(void) const
{
  return (v);
};


CP_1::operator Complex (void)
{
  return Complex( u/v  );
};



// inline functions


inline int   operator == (const CP_1& x, const CP_1& y) 
{
  return (x.numer() == y.numer() &&
          x.denom() == y.denom() );
};

inline int operator !=  (const CP_1& x, const CP_1& y) 
{
  return (x.numer() != y.numer() ||
          x.denom() == y.denom() );
};

inline CP_1 operator - (const CP_1& x) {
  return CP_1(-x.numer(), x.denom() );
}; 

inline CP_1 operator + (const CP_1& x, const CP_1& y) {
  return CP_1(x.numer()*y.denom() + y.numer()*x.denom(), 
	      x.denom()*y.denom() );
};


inline CP_1 operator - (const CP_1& x, const CP_1& y) {
  return CP_1(x.numer()*y.denom() - y.numer()*x.denom(), 
	      x.denom()*y.denom() );
};

inline CP_1 operator * (const CP_1& x, const CP_1& y) {
  return CP_1(x.numer()*y.numer(), x.denom()*y.denom() );
};
    
inline CP_1 operator / (const CP_1& x, const CP_1& y) {
  return CP_1(x.numer()*y.denom(), x.denom()*y.numer() );
};


double norm(const CP_1& x)
{
  return ( norm(x.numer()) / norm(x.denom()) );
};

double abs(const CP_1& x)
{
  return ( sqrt ( norm(x.numer()) / norm(x.denom()) ) );
};

Complex numer(const CP_1& x)
{
  return(x.numer());
};

Complex denom(const CP_1& x)
{
  return(x.denom());
};

inline CP_1 balance(const CP_1& x)
{
  double r = sqrt( norm(x.numer()) + norm(x.denom()) );
  return CP_1( x.numer()/r, x.denom()/r);
};

inline CP_1 invert(const CP_1& x)
{
  return CP_1 ( x.denom(), x.numer() );
};

inline CP_1 conj(const CP_1& x)
{
  return CP_1( conj(x.numer()), conj(x.denom()) );
};


inline ostream& operator << (ostream& s, const CP_1& x) {
  s << x.numer() << "/" << x.denom();
  return s;
};



#endif


