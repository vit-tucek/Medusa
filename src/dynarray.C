//-----------------------------------------------------------------------------
//
//             Implementering af DynArray
//
//-----------------------------------------------------------------------------

#ifndef __DYNARRAY_C
#define __DYNARRAY_C

#include "dynarray.h"


template<class T>void DynArray<T>::expand(int l)
// protected metode
//
// Beskrivelse :
// 	fordobler den interne stierrelse af arrayet, saa det kan indeholde l
//		elementer
//
// Parametre
//		(int) l	: Arrayet skal indeholde mindst l elementer
//
{
  T *tmpptr;
#ifdef	FEJL
  if (l<1 || l>maxlengde || l<lengde) throw Range();
#endif
  int nylengde = lengde;
  while (nylengde<l)
    nylengde *= 2;
  // nylengde er det mindste tal der kan skrives paa formen 2^n (n hel), og
  // opfylder at nylende >= lengde.
  tmpptr = new T[nylengde];
#ifdef	FEJL
  if (tmpptr==0) throw Memory();
#endif 
  int i = 0;
  for (i=0; i<lengde; i++) tmpptr[i] = table[i]; // det ,,gamle'' indhold
                       // af arrayet bevares
  for (i=lengde; i<nylengde;i++) tmpptr[i]=nul; 
  // nye elementer initialiseres
  delete[](table);
  table=tmpptr;
  tmpptr=0;
  lengde=nylengde;
} // expand




template<class T>ostream& operator<<(ostream& stream, const DynArray<T>& a)
// public metode
//
// Beskrivelse :
// udskrivning. Udskriver en komma separeret følge af elementer, omsluttet
// af parenteser. For eksempel kan et int DynArray udskrives som ( 5, -3, 3)
//
// Parametre :
//   (ostream&)	stream	: stroemmen der skrives til
//   (DynArray<T>) a	: det DynArray der skrives ud.
{
  stream << "(" ;
  if (a.maxassign>0) stream << a.table[0];
  for (int i=1;i<a.maxassign;i++)
    stream << ", " << a.table[i];
  stream << ") ";
  return (stream);
}  // <<


/*
ostream& operator<<(ostream& stream, DynArray<char>a)
{
  for (int i=0;(i<a.maxassign && a.table[i]!='\0');i++)
    stream << a.table[i];
  return (stream);
}; // string version of <<
*/


template<class T>DynArray<T>::DynArray(const DynArray<T>& a)
// public metode
//
// Beskrivelse :
// constructor med tilskrivning til andet DynArray. Tillader bla. at
// DynArray's overforres som værdi parametre (paspaa...).
//
// Parametre : (DynArray<T>&) a. Det nye array er identisk med a

{
  nul = a.nul;
  lengde = a.lengde;
  table = new T[lengde];
#ifdef	FEJL
  if (table == 0) throw Memory();
#endif 
  maxassign = a.maxassign;
  for (int i = 0; i < lengde; i++) table[i] = a.table[i];
} // constructor DynArray<T>(DynArray<T>&)


template<class T>DynArray<T>::DynArray(T n, int l)
// public metode
//
// Beskrivelse :
// constructor. n angiver nulelementet, l startlaengden
//
// Parametre :
//  (T)	n : angiver nul elementet. Det element der returneres hvis der
//  spoerges om utilskrevne pladser
//  (int) l : angiver start laengden
{
#ifdef	FEJL
  if ((l<1) || (l>maxlengde))
    throw Range();
#endif
  nul = n;
  lengde = 1;
  while (lengde<l)
    lengde*=2;
  // lengde er det mindste tal der kan skrives paa formen 2^n (n hel), og
  // opfylder at lende >= l. (Ja der reserveres op til dobbelt meget saa
  // meget plads som noedvendigt.
  table = new T[lengde];
#ifdef	FEJL
  if (table == 0) throw Memory();// det gik ikke godt
#endif
  for (int i = 0; i < lengde; i++) table[i]=nul; // initialisering
  maxassign = 0;
} // constructor


/*
// Streng constructor. Udkommenteret pga af linker vanskelligheder.
// Virker hvis metoden benyttes et sted i koden
DynArray<char>::DynArray(char* s)
// streng konstruktor
{
	for (int l=0; !(s[l]=='\0');l++);
	if (l==0)
		DynArray('\0', 1);
	else
	{
		nul='\0';
#ifdef	FEJL
		if (l>maxlengde) throw Range();
#endif	FEJL
		lengde=1;
		while (lengde<l)
			lengde*=2;
		table=new(char[lengde]);
#ifdef	FEJL
		if (lengde==0) throw Memory();
#endif	FEJL
		for (int i=0;i<l; i++)
			table[i]=s[i];
		for (i=l; i<lengde; i++)
			table[i]=nul;
		maxassign = l;
	};
}; // constructor, strings
*/



template<class T>T DynArray<T>::operator[] (int l)
// public metode
//
// Beskrivelse :
// Returnerer element nr l i det aktuelle DynArray. a[l] svarer til
//	a[l-1] i et konventionelt array. a[x] = vaerdi er IKKE tilladt, benyt
// assign
//
// Parametre :	(int)	l angiver hvilket element der skal returneres
{
#ifdef	FEJL
  if ( (l < 1) || (l > maxlengde) )
    throw Range();
#endif
  if (l > lengde) return(nul);
  // Dette sikrer at der kan spørges om element nr maxlengde selvom, der evt
  // kun er brugt lagerplads svarende til et element.
  return( table[l-1] );
} // []



template<class T> T DynArray<T>::assign(T e, int l)
//	public metode
//
// Beskrivelse :
//		Tilskrivning af et element. e flyttes ind paa plads l.
//
// Parametre :
//		(T)	e 	: Det element der skal lagres
//		(int) l	: pladsen hvor e lagres. 1 <= l <= maxlengde
//
// Returnerer :
//		T, e returneres
{
  if (l>lengde) expand(l);	// Hvis der ikke er plads, saa udvides
  if (l>maxassign) maxassign = l;
#ifdef	FEJL
  if (l < 1) throw Range();
#endif
  return(table[l-1] = e);
} // assign



template<class T>DynArray<T>& DynArray<T>::operator = (const DynArray<T>& a)
// public metode
//
// Beskrivelse :
//	Tilskriver et DynArray til et andet af samme type
// Bemaerk af nulelementet og maxassign ogsaa aendres, dvs saettes a = b er a
// en tro kopi af b
//
// Parametre : (const DynArray<T>)	a : Det array der kopieres
//
//	Returnerer : (DynArray<T>&) svarende til a. dvs tilskrivning af typen
//		 b = c = a ok
{
  if (this == &a) return (*this);
  // Det aktuelle array lukkes ned :
  delete[] table;
  // og et nyt (magen til a) oprettes :
  lengde = a.lengde;
  maxassign = a.maxassign;
  nul = a.nul;
  table = new T[lengde];
  for (int i = 0; i < maxassign; i++)
    table[i] = a.table[i];
  return (*this);
} // =



template<class T>int DynArray<T>::operator==(const DynArray<T>& a)
// public metode
//
// Beskrivelse :
// Tester om to DynArrays af samme type er ens, i den forstand at
// a == b, hvis og kun hvis a[n] = b[n], for alle n i N
//
// Parametre :
// 	en implicit og explicit, begge DynArrays af samme type
//
// Returnerer :
//		1 (= true) hvis de to arrays er ens (se beskrivelse)
//		0 (= false) hvis de to arrays er forskellige
{
  int min=(maxassign<a.maxassign) ? maxassign : a.maxassign;
  int i = 0;
  int ens = 1; // true
  // Foerst test pladser der er allokeret i begge arrays
  while (ens && (i<min))
    {
      ens=(table[i]==(a.table[i]));
      i++;
    };
  // hvis den virtuelle laengde er forskellig for de to arrays testes
  // mod nulelementerne
  if (ens && maxassign>a.maxassign)
    while (ens && (i<maxassign))
      {
	ens=(table[i]==a.nul);
	i++;
      }
  else if (ens && maxassign<a.maxassign)
    while (ens && (i<a.maxassign))
      {
	ens=(a.table[i]==nul);
	i++;
      };
  return (ens);
} // ==



#endif
