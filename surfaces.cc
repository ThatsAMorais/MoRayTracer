// *************************************************************************
//    file: surfaces.cpp
//  author: Alex Morais
// purpose: Sphere class derived of surface.
// created: 1-23-2007
// *************************************************************************

#include "surfaces.h"
#include "material.h"
#include <gm.h>
#include <math.h>

using namespace std;

sphere::sphere( void ){}
sphere::~sphere( void ){}

void sphere::read( std::istream& ins ){
	string cmd;                    // Buffer to hold each command
    bool   seen_end_tag  = false;  // Stop reading at end of view block
    bool   seen_position = false;  // Must see each command
	bool   seen_radius   = false;
	bool   seen_mat		 = false;

    // Loop and read until we reach the end of the view construct
    while( !ins.eof() && !seen_end_tag ) 
    {
		ins >> cmd;
		// Skip a comment line; ignore input until end-of-line		
		if( cmd == "#" ){
			ins.ignore( numeric_limits<int>::max(), '\n' ); 
		}
		// Detect end of the block
		else if( cmd == this->end_tag() ){
			seen_end_tag = true; 
		}
		// Read in commands
		else if( cmd == "position" ){			// center
			ins >> sphereCenter;
			seen_position = true;
		}
		else if( cmd == "radius" ){				// radius
			ins >> sphereRadius;
			sphereRadiusSqrd = pow(sphereRadius, 2);  // radius squared
			seen_radius = true;
		}
		else if( cmd == "begin_material" ){		// material
			// call the material's parsing func.
			mat = new material();
			mat->read( ins );
			seen_mat = true;
		}
    }

	if(!(seen_end_tag && seen_position && seen_radius && seen_mat)){
		cerr << "Error reading surfaces" << endl;
	}
}

bool sphere::intersect( ray_t& ray, double t0, double t1, hit_t& hit ){

	//
	// I - Is r.origin inside/outside the sphere//
	/////////////////////////////////////////////
	
	// oc
	gmVector3 oc = ( sphereCenter - ray.get_origin() );
	double ocLenSqrd = pow( oc.length(), 2 );	// the length of 'oc' squared
	
	bool bRayOriginOutsideSphere = false;
	
	// Is r.origin inside or outside the sphere?
	if( ocLenSqrd >= sphereRadius*sphereRadius )
		bRayOriginOutsideSphere = true;
	
	//
	// II - Determine closest approach//
	///////////////////////////////////

	// tca -> closest approach
	double tca = dot( oc, ray.get_dir_norm() );

	//
	// III - //
	////////

	// if sphereCenter is behind ray.origin
	if( bRayOriginOutsideSphere && tca < 0 ){
		hit.setT( numeric_limits<double>::infinity() );
		return false;	// --> MISS
	}

	//
	// IV //
	///////

	// thc -> halfcord distance
	double thcSqrd = sphereRadiusSqrd - ocLenSqrd + tca*tca;

	if( bRayOriginOutsideSphere && thcSqrd < 0 ){
		hit.setT( numeric_limits<double>::infinity() );
		return false;
	}

	//
	// V //
	//////

	if( bRayOriginOutsideSphere )
		hit.setT( tca - sqrt(thcSqrd) );
	else
		hit.setT( tca + sqrt(thcSqrd) );

	if( (hit.getT() > t1) || (hit.getT() < t0) )
		return false;
	
	return true;
}

void sphere::print( std::ostream& os ){

}


