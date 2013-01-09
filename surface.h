//*****************************************************************************
//    file: surface.h
//  author: J. Edward Swan II
// purpose: Surfaces are geometric entities which know how to intersect 
//          themselves with rays.  This is the most basic geometric object 
//          class.
// created: 1-30-2005
//          1-17-2007 Modified to be proper virtual base class (swan)
//
//          A surface has a 'print()' member intead of overloading '<<' because
//          '<<' is a friend function, and therefore is not properly polymorphic
//          with derived classes.
//*****************************************************************************

#ifndef SURFACE_H
#define SURFACE_H

#include <iostream>
#include "ray.h"
#include "hit.h"
#include "material.h"

class surface_t 
{
public:
  surface_t();              // Constructor
  virtual ~surface_t();     // Destructor

  // What is the name of this surface?
  virtual char* name() { return "surface"; }

  // Tags that delimit surface block
  virtual char* begin_tag();
  virtual char* end_tag();

  virtual void read( std::istream& ins );
  // requires: ins contains the commands that define the surface primitive in
  //           any order, followed by this.end_tag().  ins may contain comment
  //           lines beginning with "#".
  //  ensures: The surface primitive commands are read from ins and the view
  //           fields are set to the appropriate values.  If ins contains
  //           multiple copies of the same command, the values given by the last
  //           command instance take effect.

  virtual bool intersect( ray_t& ray, double t0, double t1, hit_t& hit );
  // requires: length( ray.dir ) != 0, t0 > 0, t1 > 0, t1 > t0
  //  ensures: IFF ray strikes this surface, THEN 
  //           this->hit( ray, t0, t1, hit ) == true, AND
  //           hit contains the relevant information

  virtual void print( std::ostream& os );
  //  ensures: A representation of the surface is printed to os.
  
  virtual gmVector3 getNormal( gmVector3 rt );
  // subclasses should define their own procedure for
  //  calculating the normal.  This will give us the
  //  normal of any surface in a general context

  material* getMaterial( void ){ return &mat; }
  // get the surfaces' material
 
protected:
  material mat;
};

#endif
