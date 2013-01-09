//*****************************************************************************
//    file: ray.h
// created: 1-30-2005
//  author: J. Edward Swan II
// purpose: Define a ray
//*****************************************************************************

#ifndef RAY_H
#define RAY_H

#include <iostream>
#include <gmVec3.h>

class ray_t 
{
	public:
		gmVector3 get_origin(void){ return origin; }
		void set_origin( gmVector3 o){ origin = o; }

		gmVector3 get_dir(void){ return dir; }	
		gmVector3 get_dir_norm(void){ return dir_norm;}
		void set_dir( gmVector3 direction );
		// requires: length( direction ) > 0
  		//  ensures: this->dir == direction, 
		//           this->dir_norm == direction / length( direction )

		gmVector3 get_p(void){ return p; }
		void set_p( double t ){ p = (origin + (t * dir_norm)); }

		//double get_refr_index( void ){ return currRefractiveIndex; }
		//void set_refr_index( double n ){ currRefractiveIndex = n; }

  		friend std::ostream& operator << ( std::ostream& os, const ray_t& );
  		//  ensures: A representation of the ray is printed to os.

	private:
		gmVector3 origin;       // Where ray starts
		gmVector3 dir;          // Direction ray points
		gmVector3 dir_norm;     // Normalized ray direction
		gmVector3 p;			// the equation of the ray
		//double currRefractiveIndex;	//the refractive index through
										// which this ray is traveling
};

inline void ray_t::set_dir( gmVector3 direction )
{ 
  assert( direction.length() > gmEPSILON );
  this->dir = direction;
  this->dir_norm = direction;
  this->dir_norm.normalize();
}

inline std::ostream& operator << ( std::ostream& os, const ray_t& r )
{
  os << "origin: " << r.origin << " dir: " << r.dir << " dir_norm: " << r.dir_norm;
  return os;
}

#endif

