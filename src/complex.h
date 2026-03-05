// -*- C++ -*- backward compatibility header.

#ifndef MEDUSA_COMPAT_COMPLEX_H
#define MEDUSA_COMPAT_COMPLEX_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <complex>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Legacy code in this project mixes integer literals with complex<double>.
// Modern std::complex templates do not accept that combination directly.
inline complex<double> operator+(const complex<double>& lhs, int rhs) {
  return lhs + static_cast<double>(rhs);
}
inline complex<double> operator+(int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) + rhs;
}
inline complex<double> operator-(const complex<double>& lhs, int rhs) {
  return lhs - static_cast<double>(rhs);
}
inline complex<double> operator-(int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) - rhs;
}
inline complex<double> operator*(const complex<double>& lhs, int rhs) {
  return lhs * static_cast<double>(rhs);
}
inline complex<double> operator*(int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) * rhs;
}
inline complex<double> operator/(const complex<double>& lhs, int rhs) {
  return lhs / static_cast<double>(rhs);
}
inline complex<double> operator/(int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) / rhs;
}

inline complex<double> operator+(const complex<double>& lhs, long int rhs) {
  return lhs + static_cast<double>(rhs);
}
inline complex<double> operator+(long int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) + rhs;
}
inline complex<double> operator-(const complex<double>& lhs, long int rhs) {
  return lhs - static_cast<double>(rhs);
}
inline complex<double> operator-(long int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) - rhs;
}
inline complex<double> operator*(const complex<double>& lhs, long int rhs) {
  return lhs * static_cast<double>(rhs);
}
inline complex<double> operator*(long int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) * rhs;
}
inline complex<double> operator/(const complex<double>& lhs, long int rhs) {
  return lhs / static_cast<double>(rhs);
}
inline complex<double> operator/(long int lhs, const complex<double>& rhs) {
  return static_cast<double>(lhs) / rhs;
}

#endif
