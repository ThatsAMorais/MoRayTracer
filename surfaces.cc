// *************************************************************************
//    file: sphere.cc
//  author: Alex Morais
// purpose: Sphere class derived of surface.
// created: 1-23-2007
// *************************************************************************

#include "surfaces.h"
#include "material.h"
#include <gm.h>
#include <math.h>
#include <string.h>
#include "functions.h"

using namespace std;

///////////// class sphere :: surface_t ////////////////////////////////////
////
///
//
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
			material* m = new material();
			m->read( ins );
			mat = *m;
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
	double tempT;
	
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
		//hit.setT( numeric_limits<double>::infinity() );
		return false;	// --> MISS
	}

	//
	// IV //
	///////

	// thc -> halfcord distance
	double thcSqrd = sphereRadiusSqrd - ocLenSqrd + tca*tca;

	if( bRayOriginOutsideSphere && thcSqrd < 0 ){
		//hit.setT( numeric_limits<double>::infinity() );
		return false;
	}

	//
	// V //
	//////

	if( bRayOriginOutsideSphere )
		tempT = tca - sqrt(thcSqrd);
	else
		tempT = tca + sqrt(thcSqrd);

	if( (tempT > t1) || (tempT < t0) ){
		return false;
	}

	// set the t value
	hit.setT( tempT );
	// calc p from the ray equation
	ray.set_p( tempT );

	// calculate the normal of the sphere
	gmVector3 vec = ray.get_p() - sphereCenter;
	vec.normalize();
	
	// store the normal in the hit
	hit.setNormal( vec );

	return true;
}

void sphere::print( std::ostream& os ){

}


///////////// class plane /////////////////////////////////////////////////
////
///
//
plane::plane( void ){}
plane::~plane( void ){}

bool plane::intersect(ray_t& ray, double t0, double t1, hit_t& hit){

	double denom = dot( ray.get_dir_norm(), normal );
	
	if( gmFuzEQ(denom, 0) ){
		return false;
	}
	
	gmVector3 p0 = FourToThree( vertices[0] );
	double t = dot( (p0 - ray.get_origin()) , normal );
	t = t / denom;

	if( t >= t0  && t < t1){

		int u, v;
		gmVector3 p = ray.get_origin() + t*ray.get_dir_norm();
			
		// determine which axis plane to flatten the surface to
		if( ( gmAbs(normal[2]) > gmAbs(normal[0]) ) && 
				( gmAbs(normal[2]) > gmAbs(normal[1]) ) )
		{
			u = 0;
			v = 1;
		}
		else if( gmAbs(normal[1]) > gmAbs(normal[0]) ){
			u = 0;
			v = 2;
		}
		else{
			u = 1;
			v = 2;
		}

		vector<gmVector3> tempVerts;
		gmVector3* aVec = NULL;

		// convert the coords of the poly to 2-space
		vertexPtrList::iterator vertexIter;
		for( vertexIter = vertices.begin();
				vertexIter != vertices.end();
				vertexIter++)
		{
			//
			gmVector4* vertPtr = (*vertexIter);
			aVec = new gmVector3( 0, 0, 0 );
			(*aVec)[0] = (*vertPtr)[u] - p[u];
			(*aVec)[1] = (*vertPtr)[v] - p[v];
	
			// push the nu vecta onda list
			tempVerts.push_back( *aVec );

			aVec = NULL;
		}

		// Begin the 2D Point-in-Polygon algo //
		// ////////////////////////////////// //

		// initialize the edge counter
		int nc = 0;
		// and the sign holder  ( nsh = vb1)
		double nsh = gmSign( (tempVerts[0])[1] );

		// iterate over the edges
		for( unsigned i = 1; i <= tempVerts.size(); i++ ){

			// the previous
			int curr,
				prev = i - 1;
			
			// wrap the first and last vertex for the final edge
			if( i == tempVerts.size() )
				curr = 0;
			else
				curr = i;

			// the endpoint coords of the current edge
			double ub = (tempVerts[curr])[0],
				   vb = (tempVerts[curr])[1],
				   ua = (tempVerts[prev])[0],
				   va = (tempVerts[prev])[1];

			double sh = nsh;
			nsh = gmSign(vb);

			if( gmFuzEQ(sh, nsh) ){
				continue; // no cross
			}

			// if the current u coord is > 0 and the previous u coord is > 0 ...
			if( (ua > 0) &&	(ub > 0) ){
				nc++;		// edge crosses the +u axis
			}
			else if( (ua < 0) && (ub < 0) ){
				continue;
			}
			else if( (ua - (va * ( (ub-ua) / (vb-va) ))) > 0 ){
				nc++;	// edge crosses the +u axis
			}
		}

		// if the remainder of divison of number of crosses over 2 is not 0
		//  (nc is odd) .... return true(intersection)
		if( (nc % 2) != 0 ){
			ray.set_p(t);
			hit.setT(t);
			hit.setNormal( normal );

			return true;
		}
	}

	return false;
}


///////////// class polygon :: surface_t ////////////////////////////////////
////
///
//
polygon::polygon( void ){}
polygon::~polygon( void ){}

void polygon::read( std::istream& ins ){
	string cmd;                    	  // Buffer to hold each command
    bool	seen_end_tag	= false;  // Stop reading at end of view block
    int		verts_seen		= 0;  	  // Must see each command
	bool	seen_poly		= false;
	bool	seen_transform	= false;
	bool    seen_mat		= false;
	gmVector4* vertex = NULL;

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
		else if( cmd == "vertex" ){			// a vertex
			vertex = new gmVector4();

			for( int i=0; i < 3; i++ ){
				ins >> (*vertex)[i];
			}

			// put a 1 in the homogeneous coordinate
			(*vertex)[3] = 1.0;

			vertices.push_back(vertex);
			verts_seen++;
		}
		else if( cmd == "poly" ){			// a poly

			// the plane for which we're gathering vertices
			plane* aPlane = new plane();

			char* line = new char[1024];
			ins.getline(line, 1024, '\n');

			// access the first token
			char* token = strtok(line, " ");
			while( true )
			{
				// 
				gmVector4* vertPtr = NULL;
				int vertIndex;

				if( token )
					vertIndex = atoi(token);
				else
					break;

				// get a pointer to the index-th vertex of
				//  this poly's list.
				if(vertIndex >= verts_seen){
					cerr << "error: bad vert index. continuing without this poly" << endl;
					delete aPlane;
					aPlane = NULL;
					ins.ignore( numeric_limits<int>::max(), '\n' );
					break;
				}

				vertPtr = vertices[vertIndex];
				aPlane->addVertex(vertPtr);

				// access the next token of the last str input
				token = strtok(NULL, " ");

			}

			// 
			if(aPlane){
				polyList.push_back(*aPlane);
				seen_poly = true;
			}
		}
		else if( cmd == "transform" ){		// transformation matrix
			ins >> transformMatrix;
			seen_transform = true;
		}
		else if( cmd == "begin_material" ){		// material
			// call the material's parsing func.
			material* m = new material();
			m->read( ins );
			mat = *m;
			seen_mat = true;
		}
    }

	if(!(seen_end_tag && ( verts_seen > 2 ) && seen_poly && seen_transform && seen_mat)){
		cerr << "Error reading material: missing attributes" << endl;
		return;
	}

	// apply the transformation to the vertices
	vertexPtrList::iterator vertexIter;
	for( vertexIter = vertices.begin();
			vertexIter != vertices.end();
			vertexIter++ )
	{
		gmVector4* vertex = (*vertexIter);
		*vertex = transformMatrix * (*vertex);
	}
	

	// calc face normals
	planeList::iterator planeIter;
	for( planeIter = polyList.begin();
			planeIter != polyList.end();
			planeIter++ )
	{
		// make 3d vectors out of the 4d vertices
		plane aPlane = (*planeIter);
		vertexPtrList vertices = *(aPlane.getVertices());
		gmVector3 p0 = FourToThree( vertices[0] ),
				  p1 = FourToThree( vertices[1] ),
				  p2 = FourToThree( vertices[2] );

		// normal = (p2-p1) x (p0-p1)
		gmVector3 normal = cross( (p2 - p1), (p0 - p1) );
		normal.normalize();
		(*planeIter).setNormal( normal );
	}	
}

bool polygon::intersect( ray_t& ray, double t0, double t1, hit_t& hit ){

	bool intersected = false;

	// iterate over the faces of this polygon
	planeList::iterator polyIter;
	for(polyIter = polyList.begin();
			polyIter != polyList.end();
			polyIter++)
	{

		// call the intersect function
		if( ((*polyIter).intersect(ray, t0, t1, hit)) ){

			intersected = true;

			t1 = hit.getT();
		}
	}

	if( intersected ){
		return true;
	}

	return false;
}

void polygon::print( std::ostream& os ){
	
}


