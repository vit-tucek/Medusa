#ifndef MEDUSA_B_H
#define MEDUSA_B_H

#include <medusa.h>
#include <fstream.h>
#include <complex.h>
#include <dynarray.C>



enum Ctype {segment, joint, other};


class Medusa_comp
{

  public :
    Medusa_comp(const CComplex& x) {z=x; next_component = 0;};
    virtual ~Medusa_comp(void) {};
    CComplex position(void);
    virtual Ctype type(void)=0;
    Medusa_comp* next_component;
    
  protected :
    CComplex z;

};  // Medusa_comp


class Segment : public Medusa_comp
{

  public :
    Segment(const CComplex& x) : Medusa_comp(x) {};
    virtual Ctype type(void);
};



class Joint : public Medusa_comp
{
  public :
    Joint(const CComplex& x, int il, int ol) : Medusa_comp(x) 
       {i_leg = il; o_leg = ol;};

    virtual Ctype type(void);

    int i_leg, o_leg;
}; // Joint



class Medusa
{
  public :
    Medusa(long int, long int, long int);
    ~Medusa(void);
    CComplex param_a(void) {return a;};
    CComplex param_b(void) {return b;};
    int iterate(void);

  protected :

    typedef struct
    {
      int legno;
      int symbol;
    } Preim;

    typedef struct
    {
      Joint* attachment_p;
      Segment* leg_p;
    } Leg;


    typedef DynArray<Preim> Preim_list;

    typedef Leg* Leg_list;

    // Data for the map f_a,b
    CComplex a;
    CComplex b;

    // the kneading data :
    int inner_preperiod;
    int inner_period;
    int outer_preperiod;
    int outer_period;

    Preim_list* i_preim1;
    Preim_list* i_preim2;
    Preim_list* o_preim1;
    Preim_list* o_preim2;

    // body of the mating spiders
    Medusa_comp* body_p;
    
    // legs
    Leg_list i_legs;
    Leg_list o_legs;

    void findorbit(long int, long int, DynArray<long int>&, int&, int&);
    void find_kneading_data(long int, int, int, DynArray<long int>&,
		     Preim_list*, Preim_list*);

    void find_succ(long int, long int, DynArray<long int>&, int&,
                   long int&, int&);
    void init_body(DynArray<long int>&, DynArray<long int>&, long int);
    void invf(CComplex, CComplex, CComplex&, CComplex&);
    void pullback_leg(int, int, Leg_list, Leg_list, CComplex&, Joint*,
		      DynArray<CComplex>&);
    void pullback(DynArray<CComplex>&);
    void find_params_and_cvs(CComplex&, CComplex&);
    int rectify(DynArray<CComplex>, CComplex&, CComplex&);
    void prune_legs(int, const Leg_list,
		    CComplex&, DynArray<CComplex>&);
    void prune(DynArray<CComplex>&, CComplex&, CComplex&);

    void circle_connect(ostream&, CComplex, CComplex, CComplex) const;

    friend ostream& operator << (ostream&, const Medusa&);
};


// misc declarations



#endif








