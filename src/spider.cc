#include <spider.h>


const ddouble infinity = 1e100;
const ddouble epsilon = 1e-100;

// #define DEBUGGING


CComplex reciprok(CComplex z)
{
  if (norm(z)>epsilon*epsilon)
    return (1/z);
  else
    return(infinity);
}



void to_sphere(CComplex u, ddouble& x, ddouble& y, ddouble& z)
  // calculates x, y and z such that (x,y,z) projects to u,
  // under stereographic projection (equator being the unit circle)
{
  if (norm(u)<epsilon)
    {
      x = y = 0;
      z = -1;
    }
  else
    {
      ddouble r = norm(u);
      z = (r-1)/(r+1);
      CComplex tmp = 2*sqrt(r)/(r+1)*u/abs(u);
      x = real(tmp);
      y = imag(tmp);
    }
}
  
    


/* ****************************************
** Definitions for the class Spider_comp
**************************************** */

CComplex Spider_comp::position(void) {
  return z;
}






/* ****************************************
** Definitions for the class Segment
**************************************** */

Ctype Segment::type(void) {
  return segment;
}

// Definitions for the class Joint

Ctype Joint::type(void) {
  return joint;
}






/* ****************************************
** // Definition for the class Couple
**************************************** */





// Protected methods ----------------------------



void Couple::findorbit(int p, int q, DynArray<int>& table, 
		  int& period, int& preperiod)
  // finds (pre)period of p/q under doubling mod 1,
  // and store orbit in table.
{
  int k = 1;
  int j = 0;
  int inlist = 0;
  
  while(!inlist) {
    table.assign(p, k);
    p *= 2;
    if (p >= q) p -= q;
    for (j=1; j<=k; j++) 
      if (table[j]==p) {
	preperiod = j-1;
	inlist = 1;
	period = k - preperiod;
      } // if and for
    k++;
  } // while 
} // findorbit




void Couple::find_kneading_data(int q, int period, int preperiod,
		     DynArray<int>& table, 
		     Preim_list* preim1, Preim_list* preim2)
  // find kneading data and stores it in preim1 and preim2
{
  // debug
  // cout << "find_kneading_data " << q << " " << period << " "
  //     << preperiod << endl;
  // cout << (table);
  int j;
  Preim temp;
  temp.legno = -1;
  temp.symbol= -1;
  if (preperiod==0)
    {
      temp.legno = period;
      temp.symbol =(2*table[period] >= q) ? 1 : 0;
    }; // if 

  preim1->assign(temp,1);
  
  for (j=2; j <= period+preperiod; j++) {
    temp.legno = j - 1;
    temp.symbol = (2*table[j-1] >= q) ? 1 : 0;
    preim1->assign(temp, j);
  }; // for


  temp.legno=-1;
  temp.symbol=-1;
  for(j=1; j <= period+preperiod; j++)
    preim2->assign(temp, j);

  if (preperiod > 0) 
    {
      temp.legno = period+preperiod;
      temp.symbol =  (2*table[preperiod+period] >= q) ? 1 : 0; 
      preim2->assign(temp, preperiod+1);
    } // if
} // find_kneading_data




void Couple::find_succ(int p, int q, DynArray<int>& table,
		int& ok, int& pp, int& legno)
{
  pp = q;
  int j;
  for (j = 1; j<=table.maxassigned(); j++)
    if (table[j]>p && table[j]<pp)
      {
	pp = table[j];
	legno = j;
      };
  ok = (pp != q);
} // find_succ





void Couple::init_body(DynArray<int>& table1, DynArray<int>& table2, int q)
{
  // Initialises the basic structure of the spiders, i.e. body and legs
  // Modifies  body_p, i_legs*, o_legs*
  // uses inner_period, inner_preperiod, outer_period, outer_preperiod

  i_legs = new Leg[inner_period+inner_preperiod];
  o_legs = new Leg[outer_period+outer_preperiod];

  int pprime =0;
  body_p = new (Segment) (1);
  Spider_comp* act_p = body_p;
  int i_ok, p1, i_legno, o_ok, p2, o_legno;
  int p = -1;
  find_succ(p, q, table1, i_ok, p1, i_legno);
  find_succ(p, q, table2, o_ok, p2, o_legno);
  
  while (i_ok || o_ok)
    {
      if (i_ok && o_ok && p1 == p2)
	// two legs.
	{
	  p = p1;
	  act_p->next_component = 
	    (Spider_comp*)(new Joint(exp(CComplex(0,2.0*M_PI*p/q)), 
			      i_legno, o_legno));
	  act_p = act_p->next_component;
	  i_legs[i_legno-1].attachment_p = (Joint*)act_p;
	  o_legs[o_legno-1].attachment_p = (Joint*)act_p;
	}
      else if (i_ok && p1 < p2 || !o_ok)
	// an inner leg
	{
	  p = p1;
	  act_p->next_component =
	    new Joint(exp(CComplex(0,2*M_PI*p/q)), i_legno, -1);
	  act_p = act_p->next_component;
	  i_legs[i_legno-1].attachment_p = (Joint*)act_p;
	}
      else
	// an outer leg
	{
	  p = p2;
	  act_p->next_component =
	    new Joint(exp(CComplex(0,2*M_PI*p/q)), -1, o_legno);
	  act_p = act_p->next_component;
	  o_legs[o_legno-1].attachment_p = (Joint*)act_p;
	}; // if else else
      find_succ(p, q, table1, i_ok, p1, i_legno);
      find_succ(p, q, table2, o_ok, p2, o_legno);
    }; // while

  act_p->next_component = body_p;

  int k;

  // initialize inner legs
  
  for(k=1; k<inner_period+inner_preperiod; k++)
    {
      i_legs[k].leg_p = 
	new Segment(0.5*exp(CComplex(0,2*M_PI*table1[k+1]/q)));
      i_legs[k].leg_p->next_component = 0;
    };
  
  i_legs[0].leg_p = new Segment(0);
  i_legs[0].leg_p->next_component = 0;

  // initialize outer legs
  
  for(k=1; k<outer_period+outer_preperiod; k++)
    {
      o_legs[k].leg_p = 
	new Segment(2.0*exp(CComplex(0,2*M_PI*table2[k+1]/q)));
      o_legs[k].leg_p->next_component = 0;
    };
  
  o_legs[0].leg_p = 
    new Segment(infinity*exp(CComplex(0, 2*M_PI*table2[1]/q)));
  o_legs[0].leg_p->next_component = 0;

} // initbody






void Couple::invf(CComplex refpos, CComplex u, CComplex& z1, CComplex& z2)
{
  if (norm(u*b-a)<epsilon*epsilon)
    if (real(refpos) > 0)
      {
        z1 = infinity;
        z2 = -infinity;
      } 
    else
      {
        z1 = -infinity;
	z2 = infinity;
      }
  else
    {
      CComplex tmp = ((b-1)*u+1-a)/(b*u-a);
      z1 = sqrt(tmp);
      z2 = -sqrt(tmp);
      // group(u, z1, z2);
      
      if (abs(abs(arg(z2/refpos)) - abs(arg(z1/refpos))) <= 1E-2)
	  cout << "Trouble, trouble!" << endl;
	     

      if (abs(arg(z2/refpos)) < abs(arg(z1/refpos)))
	{
	  tmp = z1;
	  z1 = z2;
	  z2 = tmp;
	}
    }
} // invf





void Couple::pullback_leg(int from_leg, int to_leg, 
			  Leg_list old_legs, Leg_list new_legs,
			  CComplex& startpos, Joint* attach_p, 
			  DynArray<CComplex>& dist_pts)
{
  new_legs[to_leg-1].attachment_p = attach_p;
  CComplex dummy;
  CComplex pos; 
  invf(startpos, old_legs[from_leg-1].leg_p->position(), pos, dummy);
  Segment* to_p = new Segment (pos);
  Segment* from_p = old_legs[from_leg-1].leg_p;
  new_legs[to_leg-1].leg_p = to_p;
  CComplex refpos = pos;

  while(from_p->next_component != 0)
    {
      from_p=(Segment*)(from_p->next_component);
      invf(refpos, from_p->position(), pos, dummy);
      to_p->next_component = new Segment (pos);
      refpos = pos;
      to_p = (Segment*) to_p->next_component;
    };
  dist_pts.assign(to_p->position(), dist_pts.maxassigned()+1);
  to_p->next_component=0;
} // pullback_leg






void Couple::pullback(DynArray<CComplex>& dist_pts)
{
  dist_pts.resetmaxassigned();

  Spider_comp* body_A_p = new Segment (1);
  Spider_comp* body_B_p = new Segment (-1);
  
  Spider_comp* A_p = body_A_p;
  Spider_comp* B_p = body_B_p;

  Spider_comp* old_p = body_p->next_component;

  Leg_list new_i_legs = new Leg [inner_preperiod+inner_period];
  Leg_list new_o_legs = new Leg [outer_preperiod+outer_period];

  CComplex A_pos, B_pos;

  while(old_p != body_p)
    {
      // cout << "Pulling back comp. at " << old_p->position() << endl;
      invf(A_p->position(), old_p->position(), A_pos, B_pos);
      if ( old_p->type() == segment ||
	   old_p->type() == joint && 
	   ( ((Joint*)old_p)->o_leg == -1 || 
	     ( (*o_preim1)[((Joint*)old_p)->o_leg].symbol != 0 && 
	       (*o_preim2)[((Joint*)old_p)->o_leg].symbol !=0) )
	   &&
	   ( ((Joint*)old_p)->i_leg == -1 || 
	     (*i_preim1)[((Joint*)old_p)->i_leg].symbol != 0 && 
	     (*i_preim2)[((Joint*)old_p)->i_leg].symbol != 0) 
	   )
	{
	  A_p->next_component = new Segment (A_pos);
	  // cout << "Created A-segment " << endl;
	}
      else
	{
	  // cout << "Creating A-joint "; cout.flush();
	  int newilegno, newolegno;
	  
	  if (((Joint*)old_p)->i_leg == -1)
	    newilegno = -1;
	  else
	    if ((*i_preim1)[((Joint*)old_p)->i_leg].symbol==0)
	      newilegno = (*i_preim1)[((Joint*)old_p)->i_leg].legno;
	    else if ((*i_preim2)[((Joint*)old_p)->i_leg].symbol==0)
	      newilegno = (*i_preim2)[((Joint*)old_p)->i_leg].legno;
	    else newilegno = -1;
	  
	  if (((Joint*)old_p)->o_leg == -1)
	    newolegno = -1;
	  else
	    if ((*o_preim1)[((Joint*)old_p)->o_leg].symbol==0)
	      newolegno = (*o_preim1)[((Joint*)old_p)->o_leg].legno;
	    else if ((*o_preim2)[((Joint*)old_p)->o_leg].symbol==0)
	      newolegno = (*o_preim2)[((Joint*)old_p)->o_leg].legno;
	    else newolegno = -1;
	  
	  A_p->next_component = new Joint (A_pos, newilegno, newolegno);
	  // dist_pts.assign(A_pos, dist_pts.maxassigned()+1);

          if (newilegno != -1)
	    pullback_leg(((Joint*)old_p)->i_leg, newilegno, 
			 i_legs, new_i_legs, A_pos,
			 (Joint*)A_p->next_component, dist_pts);
          if (newolegno != -1)
	    pullback_leg(((Joint*)old_p)->o_leg, newolegno, 
			 o_legs, new_o_legs, 
			 A_pos, (Joint*)A_p->next_component, dist_pts);
	  // cout << "done. " << endl;
	};

      if ( old_p->type() == segment ||
	   old_p->type() == joint && 
	   ( ((Joint*)old_p)->o_leg == -1 || 
	     ( (*o_preim1)[((Joint*)old_p)->o_leg].symbol != 1 && 
	       (*o_preim2)[((Joint*)old_p)->o_leg].symbol !=1) )
	   &&
	   ( ((Joint*)old_p)->i_leg == -1 || 
	     (*i_preim1)[((Joint*)old_p)->i_leg].symbol != 1 && 
	     (*i_preim2)[((Joint*)old_p)->i_leg].symbol != 1) 
	   )
	{ 
	  B_p->next_component = new Segment (B_pos);
	  // cout << "Created B-segment " << endl;
	}
      else
	{
	  // cout << "Creating B-joint "; cout.flush();
	  int newilegno, newolegno;
	  
	  if (((Joint*)old_p)->i_leg == -1)
	    newilegno = -1;
	  else
	    if ((*i_preim1)[((Joint*)old_p)->i_leg].symbol==1)
	      newilegno = (*i_preim1)[((Joint*)old_p)->i_leg].legno;
	    else if ((*i_preim2)[((Joint*)old_p)->i_leg].symbol==1)
	      newilegno = (*i_preim2)[((Joint*)old_p)->i_leg].legno;
	    else newilegno = -1;
	  
	  if (((Joint*)old_p)->o_leg == -1)
	    newolegno = -1;
	  else
	    if ((*o_preim1)[((Joint*)old_p)->o_leg].symbol==1)
	      newolegno = (*o_preim1)[((Joint*)old_p)->o_leg].legno;
	    else if ((*o_preim2)[((Joint*)old_p)->o_leg].symbol==1)
	      newolegno = (*o_preim2)[((Joint*)old_p)->o_leg].legno;
	    else newolegno = -1;
	  
	  B_p->next_component = new Joint (B_pos, newilegno, newolegno);
 	  // dist_pts.assign(B_pos, dist_pts.maxassigned()+1);

          if (newilegno != -1)
	    pullback_leg(((Joint*)old_p)->i_leg, newilegno, 
			 i_legs, new_i_legs, B_pos,
			 (Joint*)B_p->next_component, dist_pts);
          if (newolegno != -1)
	    pullback_leg(((Joint*)old_p)->o_leg, newolegno, 
			 o_legs, new_o_legs, 
			 B_pos, (Joint*)B_p->next_component, dist_pts);
	  // cout << " done" << endl;
	};

      old_p = old_p->next_component;
      A_p = A_p->next_component;
      B_p = B_p->next_component;
      // cout << "pulled back one component " << endl;
    }; // while
    
  // kill old body
  // cout << "Killing old body" << endl;
  Spider_comp* ptr = (old_p = body_p->next_component);
  while(ptr!=body_p)
    {
      old_p = old_p->next_component;
      delete ptr;
      ptr = old_p;
    };
  delete body_p;

  // new body
  body_p = body_A_p;
  A_p->next_component = body_B_p;
  B_p->next_component = body_A_p;
  A_p = B_p = body_A_p = body_B_p = 0;
  
  
  // Kill legs
  int j;
  for(j=0; j<inner_period+inner_preperiod; j++)
    {
      old_p = ptr = i_legs[j].leg_p;
      while(ptr!=0)
	{
	  old_p = old_p->next_component;
	  delete ptr;
	  ptr = old_p;
	}
    };
  delete [] i_legs; 
  
  for(j=0; j<outer_period+outer_preperiod; j++)
    {
      old_p = ptr = o_legs[j].leg_p;
      while(ptr!=0)
	{
	  old_p = old_p->next_component;
	  delete ptr;
	  ptr = old_p;
	}
    };
  delete [] o_legs;

  // new legs
  i_legs = new_i_legs;
  o_legs = new_o_legs;
  new_i_legs = new_o_legs = 0;

} // pullback
	  




void Couple::find_params_and_cvs(CComplex& omega1, CComplex& omega2)
// modifies a,b such that outer leg 1 ends at the critical value a/b
// and such that inner leg 1 ends at the critical value (1-a)/(1-b)
// After completion of routine, omega1 and omega2 holds the crit. values
// a/b and (1-a)/(1-b) resp.
{
  Spider_comp* ptr = o_legs[0].leg_p;

  // We find omega1:
  while(ptr->next_component != 0)
    ptr = ptr->next_component;
  omega1 = ptr->position();

  // Then omega2
  ptr = i_legs[0].leg_p;
  while(ptr->next_component != 0)
    ptr = ptr->next_component;
  omega2 = ptr->position();
  
  // Finally we modify a and b
  b = (omega2-1)/(omega2-omega1);
  a = omega1*b;
} // find_params_and_cvs





int Couple::rectify(DynArray<CComplex> dist_pts, 
		     CComplex& omega1, CComplex& omega2)
  // Rectifies the spider, adding intermidiate segments if necessary
  // Returns 1 if succesful
{
  try 
    {
      const int M = 4000; // Maximal number of extra body/leg-parts we can
      // ad before failing
      
      int added_parts = 0;
      
      
      // Rectifying the body
      // cout << "(Rectifying body "; 
      // cout.flush();
      Spider_comp* tmp_p;
      Spider_comp* ptr = body_p;
      CComplex mid_pos;
      do
	{
	  while(!is_homotopic(ptr->position(), ptr->next_component->position(),
			      omega1, dist_pts, mid_pos) 
		&& added_parts < M)
	    {
	      tmp_p = new Segment (mid_pos);
	      tmp_p->next_component = ptr->next_component;
	      ptr->next_component = tmp_p;
	      added_parts++;
	    };
	  ptr = ptr->next_component;
	} while (ptr != body_p);
      // cout << "done) "; cout.flush();
      
      // Rectifying the inner legs
      int j;
      for(j=0; j<inner_period+inner_preperiod; j++)
	{
	  CComplex att_pos = i_legs[j].attachment_p->position();
	  while(!is_homotopic(att_pos, i_legs[j].leg_p->position(), 
			      omega1, dist_pts, mid_pos)
		&& added_parts < M)
	    {
	      tmp_p  = new Segment (mid_pos);
	      tmp_p->next_component = i_legs[j].leg_p;
	  i_legs[j].leg_p = (Segment*)tmp_p;
	  added_parts++;
	    };
	  ptr = i_legs[j].leg_p;
	  while(ptr->next_component != 0)
	    {
	      while(!is_homotopic(ptr->position(), 
				  ptr->next_component->position(),
				  omega1, dist_pts, mid_pos)
		    && added_parts < M
		    )
		{
		  tmp_p = 
		    new Segment (mid_pos);
		  tmp_p->next_component = ptr->next_component;
		  ptr->next_component = tmp_p;
		  added_parts++;
		};
	      ptr = ptr->next_component;
	    }
	};
      
      // We now have to deal with the outer legs, and perform a change of
      // variables u = 1/z;
      
      for(j=1; j<=dist_pts.maxassigned(); j++)
	dist_pts.assign(reciprok(dist_pts[j]), j);
      
      for(j=0; j<outer_period+outer_preperiod; j++)
	{
	  CComplex att_pos = reciprok(o_legs[j].attachment_p->position());
	  while(!is_homotopic(att_pos, reciprok(o_legs[j].leg_p->position()), 
			      reciprok(omega2), dist_pts, mid_pos)
	    && added_parts < M)
	    {
	      tmp_p  = new Segment (reciprok(mid_pos));
	      tmp_p->next_component = o_legs[j].leg_p;
	      o_legs[j].leg_p = (Segment*)tmp_p;
	      added_parts++;
	    };
	  
	  ptr = o_legs[j].leg_p;
	  while(ptr->next_component != 0)
	    {
	      while(!is_homotopic(reciprok(ptr->position()), 
				  reciprok(ptr->next_component->position()),
				  reciprok(omega2), dist_pts, mid_pos)
		    && added_parts < M
		    )
		{
		  tmp_p = 
		    new Segment (reciprok(mid_pos));
		  tmp_p->next_component = ptr->next_component;
		  ptr->next_component = tmp_p;
		  added_parts++;
		};
	      ptr = ptr->next_component;
	    }
	};
      return (added_parts < M);
    }
  catch (Corruption)
    {
      return 0;
    }
} // Rectify




void Couple::prune_legs(int nooflegs, const Leg_list legtable, 
			CComplex& omega,
			DynArray<CComplex>& dist_pts)
{
 int j;
 Spider_comp* tmp_p;
  for(j=0; j<nooflegs; j++)
    {
      CComplex att_pos = legtable[j].attachment_p->position();
      Spider_comp* ptr = legtable[j].leg_p;
      while(ptr->next_component != 0 && 
	    can_prune(att_pos, ptr->position(), 
		      ptr->next_component->position(), omega, dist_pts))
	{
	  legtable[j].leg_p = (Segment*)ptr->next_component;
	  delete ptr;
	  ptr = legtable[j].leg_p;
	}
      
      while(ptr->next_component != 0 &&
	    ptr->next_component->next_component != 0)
	{
	  while(ptr->next_component->next_component != 0 &&
		can_prune(ptr->position(), ptr->next_component->position(),
			  ptr->next_component->next_component->position(),
			  omega, dist_pts))
	    {
	      tmp_p = ptr->next_component;
	      ptr->next_component = ptr->next_component->next_component;
	      delete tmp_p;
	    };
	  ptr = ptr->next_component;
	} // while
    } // for
} // prune legs




void Couple::prune(DynArray<CComplex>& dist_pts,
		   CComplex& omega1, CComplex& omega2)
  // Remove unnecessary segments
{
  // First from body
  Spider_comp* ptr = body_p;
  Spider_comp* tmp_p;
  do
    {
      while (ptr->next_component != body_p &&
	     ptr->next_component->type()==segment &&
	  can_prune(ptr->position(), ptr->next_component->position(),
		    ptr->next_component->next_component->position(),
		    omega1, dist_pts))
	{
	  tmp_p = ptr->next_component;
	  ptr->next_component = ptr->next_component->next_component;
	  delete tmp_p;
	};
      ptr = ptr->next_component;
    } while (ptr != body_p);

  // inner legs
  prune_legs(inner_period+inner_preperiod, i_legs, omega1, dist_pts);

  // outer legs
  prune_legs(outer_period+outer_preperiod, o_legs, omega2, dist_pts);

} // prune








void Couple::circle_connect(ostream& s, CComplex a, 
			    CComplex b, CComplex omega) const
{
  const int n = 20;

  CComplex c;
  ddouble r;
  CComplex average;
  find_circle(a,b, omega, c, r, average);
  if (!(r<infinity))
    {
      // the points are connected by a line
      int j;
      for(j = 1; j<=n; j++ )
	  s << ",[" << a+(b-a)*j/n << "]";
    }

  else
    {
      // we draw an arc of a circle between a and b
      ddouble theta_a = arg(a-c);
      ddouble theta_b = arg(b-c);
      if (theta_b < theta_a)
	theta_b+=2.0*M_PI;
      
      ddouble theta_o = arg(omega-c);
      if (theta_o < theta_a)
	theta_o += 2.0*M_PI;
      
      int j;
      ddouble new_arg;
      for(j=1; j<=n; j++)
	{
	  new_arg = (theta_b < theta_o) ? 
	    (1.0*j*(theta_b - theta_a))/n+theta_a :
	    theta_a-1.0*j*(2*M_PI+theta_a-theta_b)/n;
	  s << ",[" << c+polar(r,new_arg) << "]";
	}
    } // else
} // circle_connect
  



// public methods  -----------------------------------




int Couple::iterate(void)
{
  int ok;

  ofstream debug_f;

  DynArray<CComplex> dist_pts(0);

  debug_f.open("before_pullback_txt");
  line_output_body(*this, debug_f);
  debug_f.close();



  pullback(dist_pts);
  CComplex omega1, omega2;

  // cout << "Calling find_params_and_cvs...";
  find_params_and_cvs(omega1, omega2);
  // cout << "done." << endl;

  // debug_f.open("before_rect_txt");
  // line_output_body(*this, debug_f);
  // debug_f.close();


  // cout << "Calling rectify with the following medusa";
  // cout << (*this) << endl;
  // cout.flush();
  
  ok = rectify(dist_pts, omega1, omega2);
  // cout << "done." << endl;

  
  // debug_f.open("after_rect_txt");
  // line_output_body(*this, debug_f);
  // debug_f.close();



  // cout << "Medusa BEFORE pruning: " << endl;
  // cout << (*this) << endl;
  // cout << "(Calling prune "; cout.flush();
  prune(dist_pts, omega1, omega2);


  // debug_f.open("after_prune_txt");
  // line_output_body(*this, debug_f);
  // debug_f.close();

  return ok;
};



Couple::Couple(int p1, int p2, int q) 
{  
  // Initialises a p1/q, p2/q  Medusa.

  // Initialize map
  a = 1;
  b = 0;

  p1 = q - p1; // Inner legs orientation is reversed
  // figure out periods, and preperiods
  DynArray<int> table1(0);
  findorbit(p1, q, table1, inner_period, inner_preperiod);
  DynArray<int> table2(0);
  findorbit(p2, q, table2, outer_period, outer_preperiod);

  #ifdef DEBUGGING
  cout << "Period is " << inner_period 
       << " Preperiod is " << inner_preperiod << endl;
  cout << "Period is " << outer_period << " Preperiod is " 
       << outer_preperiod << endl;
  #endif

  // store kneading data

  int j,k;
  Preim temp;
  temp.legno = -1;
  temp.symbol= -1;
  i_preim1 = new (Preim_list)(temp);
  i_preim2 = new (Preim_list)(temp);
  o_preim1 = new (Preim_list)(temp);
  o_preim2 = new (Preim_list)(temp);

  find_kneading_data(q, inner_period, inner_preperiod,
		     table1, i_preim1, i_preim2);

  find_kneading_data(q, outer_period, outer_preperiod,
  		     table2, o_preim1, o_preim2);

#ifdef DEBUGGING
  for (k = 1; k<= i_preim1->maxassigned(); k++)
  {
    cout << ((*i_preim1)[k].legno) << " ";
    cout << ((*i_preim1)[k].symbol) << endl;
  };

  cout << endl;


  for (k = 1; k<= i_preim2->maxassigned(); k++)
  {
    cout << ((*i_preim2)[k].legno) << " ";
    cout << ((*i_preim2)[k].symbol) << endl;
  };
   
  cout << endl;

  for (k = 1; k<= o_preim1->maxassigned(); k++)
  {
    cout << ((*o_preim1)[k].legno) << " ";
    cout << ((*o_preim1)[k].symbol) << endl;
  };

  cout << endl;


  for (k = 1; k<= o_preim2->maxassigned(); k++)
  {
    cout << ((*o_preim2)[k].legno) << " ";
    cout << ((*o_preim2)[k].symbol) << endl;
  };
  
  cout << endl;
#endif

  // initialize body and legs

  init_body(table1, table2, q);

} 
 



Couple::~Couple(void)
{
  // Deletes the Medusa
  // kill body
  Spider_comp* old_p = body_p->next_component;
  Spider_comp* ptr = old_p;
  while(ptr!=body_p)
    {
      old_p = old_p->next_component;
      delete ptr;
      ptr = old_p;
    };
  delete body_p;

  // Kill legs
  int j;
  for(j=0; j<inner_period+inner_preperiod; j++)
    {
      old_p = ptr = i_legs[j].leg_p;
      while(ptr!=0)
	{
	  old_p = old_p->next_component;
	  delete ptr;
	  ptr = old_p;
	}
    };
  delete [] i_legs; 
  
  for(j=0; j<outer_period+outer_preperiod; j++)
    {
      old_p = ptr = o_legs[j].leg_p;
      while(ptr!=0)
	{
	  old_p = old_p->next_component;
	  delete ptr;
	  ptr = old_p;
	}
    };
  delete [] o_legs;
} // ~Couple(void)

  

ostream& operator << (ostream& s, const Couple& c)
{
  ddouble x,y,z;

  s << "a:=" << real(c.a) << "+I*(" << imag(c.a)  
    << "); b:=" << real(c.b) << "+I*(" << imag(c.b) 
    << ");" << endl;
  Spider_comp* comp_p = c.body_p;

  // first we are outputting in 3D:

  s << "spacecurve({[";
  to_sphere(c.body_p->position(), x,y,z);
  s << "[" << x << "," << y << "," << z << "]";
  comp_p = comp_p->next_component;
  while (comp_p != c.body_p)
    {
      to_sphere(comp_p->position(), x,y,z);
      s << ",[" << x << "," << y << "," << z  << "]";
      comp_p = comp_p->next_component;
    }; // while
  s << ",[1,0,0]]";

  int j; Segment* l_p;
  for(j=0; j<c.inner_period+c.inner_preperiod; j++)
    {
      l_p = c.i_legs[j].leg_p;
      to_sphere(c.i_legs[j].attachment_p->position(), x,y,z);
      s << ",[["  << x << "," << y << "," << z << "]";
      while(l_p != 0)
	{
	  to_sphere(l_p->position(), x,y,z);
	  s << ",[" << x << "," << y << "," << z << "]";
	  l_p = (Segment*) l_p->next_component;
	}; // while
      s << "]";
    }; // for

  for(j=0; j<c.outer_period+c.outer_preperiod; j++)
    {
      l_p = c.o_legs[j].leg_p;
      to_sphere(c.o_legs[j].attachment_p->position(), x,y,z);
      s << ",[[" << x << "," << y << "," << z << "]";

      while(l_p != 0)
	{
	  to_sphere(l_p->position(), x,y,z);
	  s << ",[" << x << "," << y << "," << z << "]";
	  l_p = (Segment*) l_p->next_component;
	}; // while
      s << "]";
    }; // for

  s << "}, orientation=[0,65]);" << endl;




  // Now we are outputting in the plane (stereographic projection)
  CComplex omega1 = c.a/c.b;
  CComplex omega2 = (1 - c.a)/(1 - c.b);

  s << "p1 := plot({[";
  s << "[" << (c.body_p->position()) << "]" << endl;

  CComplex last_pos = c.body_p->position();

  comp_p = comp_p->next_component;
  do 
    {
      c.circle_connect(s, last_pos, comp_p->position(), omega1);
      last_pos = comp_p->position();
      comp_p = comp_p->next_component;

    } while (comp_p != c.body_p->next_component);
  s << "]"; // ",[(1,0)]]";

  for(j=0; j<c.inner_period+c.inner_preperiod; j++)
    {
      l_p = c.i_legs[j].leg_p;
      s << ",[[" << c.i_legs[j].attachment_p->position() << "]";
      last_pos = c.i_legs[j].attachment_p->position(); 
      while(l_p != 0)
	{
	  if (norm(l_p->position())<25)
	    {
	      c.circle_connect(s, last_pos, l_p->position(), omega1);
	      last_pos = comp_p->position();
	    }
	  l_p = (Segment*) l_p->next_component;
	}; // while
      s << "]";
    }; // for

  for(j=0; j<c.outer_period+c.outer_preperiod; j++)
    {
      l_p = c.o_legs[j].leg_p;
      s << ",[[" << c.o_legs[j].attachment_p->position() << "]";
      last_pos = c.o_legs[j].attachment_p->position(); 

      while(l_p != 0)
	{
	  if (norm(l_p->position())<25)
	    {
	      c.circle_connect(s, last_pos, l_p->position(), omega2);
	      last_pos = l_p->position();
	    }
	  l_p = (Segment*) l_p->next_component;
	}; // while
      s << "]";
    }; // for

  s << "}, color = blue):" << endl;

  s << "p2 := plot ([[" << (c.a/c.b) << "], [" 
    << ((1-c.a)/(1-c.b)) << "]], style = point, symbol=CROSS):" << endl;
  s << "display ([p1, p2], axes=BOXED);" << endl;

  return s;
};






void debug_output (const Couple& c)
{
  ddouble x,y,z;

  Spider_comp* comp_p = c.body_p;

  // first we are outputting in 3D:

  cout << c.body_p->position();
  comp_p = comp_p->next_component;
  while (comp_p != c.body_p)
    {
      cout << comp_p->position();
      comp_p = comp_p->next_component;
    }; // while

  int j; Segment* l_p;
  cout << endl << "Nooooow...Inner legs!!!" << endl;
  for(j=0; j<c.inner_period+c.inner_preperiod; j++)
    {
      l_p = c.i_legs[j].leg_p;
      cout << c.i_legs[j].attachment_p->position();
      while(l_p != 0)
	{
          cout << l_p->position();
	  l_p = (Segment*) l_p->next_component;
	}; // while
    }; // for

  cout << endl << "Nooooow...Outer legs!!!" << endl;
  for(j=0; j<c.outer_period+c.outer_preperiod; j++)
    {
      l_p = c.o_legs[j].leg_p;
      cout << c.o_legs[j].attachment_p->position();

      while(l_p != 0)
	{
          cout << l_p->position();
	  l_p = (Segment*) l_p->next_component;
	}; // while
    }; // for
}


void line_output_body (const Couple& c, ostream& s)
{
  CComplex a = c.a;
  CComplex b = c.b;
  CComplex u, v, z;


  Spider_comp* comp_p = c.body_p;
  s << endl << "p1 := plot([";
  u = c.body_p->position();
  z = ((b-1)*u+1-a)/(b*u-a);
  s << "[" << z << "]";
  comp_p = comp_p->next_component;
  while (comp_p != c.body_p)
    {
      u = comp_p->position();
      z = ((b-1)*u+1-a)/(b*u-a);
      s << ",[" << z << "]";
      comp_p = comp_p->next_component;
    }; // while
  u = c.body_p->position();
  z = ((b-1)*u+1-a)/(b*u-a);
  s << ",[" << z << "]], ";
  s << "colour = blue):" << endl;
  
  int j;
  s << "p2 := plot([";

  u = c.i_legs[0].leg_p->position();
  z = ((b-1)*u+1-a)/(b*u-a);
  s << "[" << z << "]";
  for (j=1; j<c.inner_period+c.inner_preperiod; j++)
    {
      u = c.i_legs[j].leg_p->position();
      z = ((b-1)*u+1-a)/(b*u-a);
      s << ",[" << z << "]";
    }

  for (j=0; j<c.outer_period+c.outer_preperiod; j++)
    {
      u = c.o_legs[j].leg_p->position();
      z = ((b-1)*u+1-a)/(b*u-a);
      if (norm(z) <= 10)
	s << ",[" << z << "]";
    }

  s << "], style = point, symbol = CROSS, colour = red):" << endl;
  s << "display([p1, p2], axes=boxed);" << endl;

}


/*
void main(void) {
	
//	DynArray<CComplex> dist (0);
//	CComplex mid;

//	dist.assign (CComplex (0.101622,0.0239985), 1);

//	is_homotopic (CComplex (0.128455,0.018985), CComplex (0.101033,0.024138), CComplex (-0.418114,0.19745), dist, mid);
//	exit (0);
	

  int p1, p2, q1, q2;

  cout << "Enter p1 : ";
  cin >> p1;
  cout << "Enter q1 : ";
  cin >> q1;
  cout << "Enter p2 : ";
  cin >> p2;
  cout << "Enter q2 : ";
  cin >> q2;
  
  Couple c(p1*q2, p2*q1, q1*q2);
  ofstream f, params;
  f.open("medusa_txt");
  f << c;
  f.close();

  params.open("parameters_txt");
  
  char r;

  cout << "Type y to iterate, to iterate saving output, and q to quit ";
  cin >> r;
  
  while (r=='y' || r=='o')
    {
      c.iterate();
      if (r == 'o')
	{
	  f.open("medusa_txt");
	  f << c;
	  f.close();
	}
      cout << "a=" << c.param_a() << " b=" << c.param_b() << endl;
      params << "/ " << c.param_a() << " " << c.param_b() << endl;
      cout << "Type y/o to iterate again ";
      cin >> r;
    }
  params << '*' << endl;
  params.close();
} // main

*/


