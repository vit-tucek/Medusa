// Dynamisk array
// (c) Christian Henriksen 07.04.95
//
// En arraystruktur, hvor antallet af elementer ikke er fastsat ved oprettelses
// tidspunktet.
// Elementerne kan lagres og hentes paa plads nr 1 og opad (der er ingen plads
// nr. 0, foerste element er element nr. 1)
// Ved oprettelsen skal angives et nul element. Spoerges om et element der ikke
// er tilskrevet noget returneres nul elementet.
//
// Tidsforbrug :
//     WORST CASE KOMPLEXITET:
// initialisation : O(n)
// []             : O(1)
// assign(a,n)    : O(n)
// ==             : O(max(n,m)) (m is length of the DynArray being compared to)
// =              : O(m) (m is length of the DynArray being copied to)
//
// Bemaerk at selvom worst case kompleksiten for assign(a, n) er O(n), er
// kompleksiteten kun O(n) for at tilskrive n elementer fra plads 1 til n 
// (i denne situation altsaa det samme som et konventionelt array og ikke 
// O(n*n) som man kunne frygte).
//
// Lager forbrug :
//	Saet m til det hoejeste index der er tilskrevet i det dynamiske array 
//      af type T
//	da bruges hoejst 2*m*sizeof(T), altsaa hoejst det dobblete af det 
//      et konventionelt array ville bruge
//
// Bemaerkninger :
// Hvis a, b og c er dynamiske arrays af samme type er a = b = c tilskrivning
//	muligt
//
//  = operatoren antager at objekterne allerede er initialiserede
//
//	Maengden af brugt lagerplads kan kun stige, indtil arrayet bliver 
//      "destructed"
//
// Elementerne skal vaere af en type T der kan skrives ud, dvs <<(ostream&, T)
// skal vaere defineret
//
// Streng behandlings faciliteter er udkommenteret og brug af header fil slaaet
// fra pga linker problemer.



#ifndef DYNA_CPP
#define DYNA_CPP


#include <iostream.h>    // pga cout and <<



const int maxlengde = 32000; // maximum antal elements en DynArray kan indeholde



template<class T> class DynArray
{
  protected :
  
  int lengde;	// antallet af reserverede lager pladser
  
  int maxassign;
  // max index af bruger tilskrevet element (maxassign <= lengde)
  
  T nul; // nul elementet

  T* table; // i dette array lagres elementerne
  
  void expand(int l);
  // fordobler stoerrelsen af reserveret lager
  
  public :
  
  class Range { };
  // exception, kastes hvis der bedes om et ikke positivt element., 
  // eller et med index stoerre end maxlengde
  
  class Memory { };
  // exception, kastes hvis der ikke kan reserveres lager
  
  DynArray(DynArray<T>& a);
  // constructor med tilskrivning til andet DynArray. Tillader bla. at
  // DynArray's overfoeres som vaerdi parametre (paspaa...).
  
  DynArray(T n, int l = 1);
  // constructor. n angiver nulelement, l startlaengde af arrayet.
  
  // DynArray(char* s);
  // speciel string constructor
  
  ~DynArray(void) { delete[] (table);}
  // Destructoren.
  
  void resetmaxassigned(void) {maxassign = 0;}
  // Der begyndes forfra med at holde styr på, max index af tilskrevne
  // elementer. Arrayet opfoerer sig som om alle elementer er sat til
  // nul elementet.
  
  T operator [] (int l);
  // Index operatoren. Returnerer indholdet af plads nr l. Hvis det ikke
  // er tilskrevet noget returneres nulelementet.
  
  T assign(T e, int l);
  // Tilskrivning. tabel.assign(e, l) svarer til tabel[l-1] = e i et konven-
  // tionelt array
  
  int maxassigned(void) {return(maxassign);}
  // Returnerer den det maximale index paa elementer der er tilskrevne (med
  // assign) siden sidste resetmaxassigned
  
  DynArray<T>& operator=(const DynArray<T>& a);
  // Tilskrivning af et DynArray til et andet af samme type. Alle
  // elementer kopieres
  
  int operator==(const DynArray<T>& a);
  // Tester om to DynArrays af samme type er ens, i den forstand at
  // a == b, hvis og kun hvis a[n] = b[n], for alle n i N
  
  // friend ostream& operator<<(ostream& stream, DynArray<char> a);
  // speciel string output
  
friend ostream& operator<<(ostream& stream, DynArray<T> a);
  // Standard (vektor stil) output
};

//ostream& operator<<(ostream& stream, DynArray<char> a);
// special string output

#endif

