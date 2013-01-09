// *************************************************************************
//    file: surfaces.h
//  author: Alex Morais
// purpose: Sphere class derived of surface.
// created: 1-23-2007
// *************************************************************************

#ifndef SURFACES_H
#define SURFACES_H

#include <iostream>
#include <gm.h>

using namespace std;

#include "surface.h"

// a 'sphere' class.  it has a radius and center.
class sphere : public surface_t{
	public:
		// Constructor/Destructor
		sphere();
		~sphere();

		char* begin_tag(void){ return "begin_sphere"; }
		char* end_tag(void){ return "end_sphere"; }

		// Other necessities from the base class
		char* name() { return "sphere"; }
		// Over-ridden virtual funcs
		void read( std::istream& ins );
		bool intersect( ray_t& ray, double t0, double t1, hit_t& hit );
		void print( std::ostream& os );

		double getRadius( void ){ return sphereRadius; }
		double getRadiusSquared( void ){ return sphereRadiusSqrd; }
		gmVector3 getCenter( void ){ return sphereCenter; }

	protected:
		// mathematical components
		double sphereRadius,
			   sphereRadiusSqrd;
		gmVector3 sphereCenter;

		// surface's material 
		//material mat			(inherited from surface_t)
};

#endif
