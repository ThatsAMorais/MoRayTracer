// *************************************************************************
//    file: scene.cpp
//  author: Alex Morais
// purpose: Scene class to store all objects in the scene and lights
// created: 1-24-2007
// *************************************************************************

//#include "functions.h"
#include "scene.h"
#include "surfaces.h"
#include "functions.h"
#include <gmMat4.h>
//#include <math.h>

using namespace std;

///////////// Light /////////////////
Light::Light(){}
Light::~Light(){}

/*
void Light::setVector( gmVector3 vec ){
	if( identity == DIR_LIGHT )
		direction = vec;
	else if( identity == POS_LIGHT )
		position = vec;
}
*/

gmVector3 Light::getVector( void ){

	if( identity == DIR_LIGHT )
		return direction;	
	else
		return position;
}


void Light::setColor( gmVector3 col ){
	color = col;
}

gmVector3 Light::getColor( void ){
	return color;
}

void Light::read( std::istream& ins ){
		
	std::string cmd;        // Buffer to hold each command
	bool   seen_end_tag  = false; // Stop reading at end of view block
	bool   seen_color = false;    // Must see each command
	bool   seen_vec = false;
	
	// Loop and read until we reach the end of the view construct
	while( !ins.eof() && !seen_end_tag ){
		//
		ins >> cmd;
		//
		
		// Skip a comment line; ignore input until end-of-line		
		if( cmd == "#" ){
			ins.ignore( std::numeric_limits<int>::max(), '\n' ); 
		}
		// Detect end of the block
		else if( cmd == this->end_tag() ){
			seen_end_tag = true; 
		}
		else if( cmd == "position" ){
			ins >> position;
			identity = POS_LIGHT;
			seen_vec = true;
		}
		else if( cmd == "direction" ){
			ins >> direction;
			identity = DIR_LIGHT;
			seen_vec = true;
		}
		// Read in commands
		else if( cmd == "color" ){			// color
			ins >> color;
			seen_color = true;
		}
	}	
}

/////////////////////////////////////////////////


//////////////////// Scene //////////////////////
Scene::Scene(){}
Scene::~Scene(){}

void Scene::read( istream& ins ){
	string cmd;                   // Buffer to hold each command
    bool   seen_end_tag = false;  // Stop reading at end of view block
    bool   seen_bg      = false;  // Must see each command
	bool   seen_a_surface = false;
	surface_t* surface = NULL;
	Light* light = NULL;

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
		else if( cmd == "background" ){ 
			ins >> background1;
			seen_bg = true;
		}
		else if(cmd == "background2" ){
			ins >> background2;
			hasTwoBGColors = true;
		}
		else if( cmd == "begin_sphere" ){
			surface = new sphere();

			// call the sphere's parsing func.
			surface->read( ins );

			// push the new sphere into the surface list
			surfacesInScene.push_back(surface);

			seen_a_surface = true;
			surface = NULL;
		}
		else if( cmd == "begin_poly" ){
			surface = new polygon();

			// parse the polygon
			surface->read( ins );

			// push the new poly into the list
			surfacesInScene.push_back(surface);

			seen_a_surface = true;
			surface = NULL;
		}
		else if( cmd == "begin_light" ){
			light = new Light();
			light->read( ins );
			lightsInScene.push_back(light);
		}
	}

    // Make sure we've seen the background
    if(!seen_bg){
		cerr << "Error: did not see a background command in scene block!" << endl;
    }

	if(!hasTwoBGColors)
		currentBG = background1;

	if(!seen_a_surface)
		cerr << "Did not see any surfaces in the script." << endl;
}
/////////////////////////////////////////////////

surface_t* Scene::checkIntersections(ray_t& ray, double t0, double t1, hit_t& hit, 
		double time, bool checkOne)
{	

	list<surface_t*>::iterator surfaceIter;
	surface_t* nearestSurface = NULL;

	/* For every object in scene */
	for(surfaceIter = surfacesInScene.begin(); 
			surfaceIter != surfacesInScene.end();
			surfaceIter++)
	{

		/* If ray intersects this object { */
		surface_t* hitObject = (*surfaceIter)->intersect(ray, t0, t1, hit, time);

		if( hitObject != NULL ){

			//cerr << "we got a hit" << endl;

			// ---Optimization---
			// for checking a single intersection
			//  when you don't need the closest t
			if(checkOne){
				return hitObject;
			}

			t1 = hit.getT();
			nearestSurface = hitObject;
		}
	}

	if( nearestSurface )
		return nearestSurface;

	return NULL;
}

void Scene::calcNextBGColor( int y, int numYPixels ){
	// this function calculates the background color as a ratio to rows rendered
	// DOESN'T WORK RIGHT
	
	// so, if y = 0 then background1 will be the color
	// and if y = numYPixels then background2 will be the color
	// otherwise bgColor = ( (1-ratio)*bg1 + ratio*bg2 ) / 2
	double ratio = y / numYPixels;
	currentBG[0] = ( background1[0]*( 1.0-ratio ) )/2 + ( background2[0]*ratio )/2;
	currentBG[1] = ( background1[1]*( 1.0-ratio ) )/2 + ( background2[1]*ratio )/2;
	currentBG[2] = ( background1[2]*( 1.0-ratio ) )/2 + ( background2[2]*ratio )/2;

	currentBG = currentBG / 2;
}

gmVector3 Scene::calcTexColor(surface_t* surface, ray_t& ray, hit_t& hit){

	gmVector3 matColor;
	material* surfaceMat = surface->getMaterial();

	char* surfaceType = surface->name();
	
	// if the object is a sphere
	if( ! strcmp( surfaceType, "sphere" ) ){
		
		gmVector3 center = ((sphere*)surface)->getCenter();
		gmVector3 p = ray.get_p();

		// do some shit here ( map the (x,y,z) of hit to (u,v) of image )
		double theta = acos( (p[2]-center[2]) / ((sphere*)surface)->getRadius() ),
			   phi = atan2( p[1]-center[1], p[0]-center[0] );

		if( theta < 0 )
			theta = 0;
		else if( theta > gmPI )
			theta = gmPI;

		// clamp the angles
		if(phi < -gmPI)
			phi = -gmPI;
		else if( phi > gmPI )
			phi = gmPI;

		if( phi < 0 )
			phi += 2*gmPI;

		double u = ( phi + gmPI )/ (2 * gmPI),
			   v = (gmPI - theta)/gmPI;

		// get the color from the texture
		matColor = surfaceMat->getColor(u,v);
	}
	else if( ! strcmp( surfaceType, "polygon" ) ){
 
		// get the side of the poly that was hit
		plane hitFace = ((polygon*)surface)->getFace(hit.getHitFaceIndex());

		if(hitFace.getNumVertices() != 4)
			matColor = surfaceMat->getColor();
		else{

			gmVector4 p0 = hitFace.getVertex(0),
					  oneMinusZero = hitFace.getVertex(1) - p0,
					  threeMinusZero = hitFace.getVertex(3) - p0;

			gmVector3 p = ray.get_p(),
					  M = p - FourToThree( &p0 ),
					  U = FourToThree( &oneMinusZero ),
					  V = FourToThree( &threeMinusZero );
			
			double u = ( dot(M,U) / pow( U.length(), 2 ) );
			double v = ( dot(M,V) / pow( V.length(), 2 ) );

			matColor = surfaceMat->getColor(u,v);
		}
		
	}
	else{
		cerr << "No surface type!" << endl;
		matColor = surfaceMat->getColor();
	}

	return matColor;
}

gmVector3 Scene::calcPhongColor(material* surfaceMat, ray_t& ray, hit_t& hit,
		gmVector3 matColor, double time, gmVector3 eye){

	/////////////
	// AMBIENT //
	/////////////

	// ambient term
	gmVector3 ambient;
	ambient[0] = ( surfaceMat->getAmbient()[0] * matColor[0] );
	ambient[1] = ( surfaceMat->getAmbient()[1] * matColor[1] );
	ambient[2] = ( surfaceMat->getAmbient()[2] * matColor[2] );

	// Add the ambient light to the final color.
	//  (Obligatory)
	gmVector3 phongColor = ambient;
	/////////////
	
	// Next we iterate over the lights 
			
	// light list iterator ...
	list<Light*>::iterator lightIter;

	// p => from the ray equation
	gmVector3 p = ray.get_p();
	gmVector3 normal = hit.getNormal();

	// ... used here.
	for(lightIter = lightsInScene.begin();
			lightIter != lightsInScene.end();
			lightIter++)
	{
		// Get the lights color(convenience)
		gmVector3 lightCol = (*lightIter)->getColor();

		// get id of the light (directional vs. positional)
		int id = (*lightIter)->getIdentity();

		// the "l" vector
		gmVector3 l;

		// determie lVec depending on light type
		if( id == DIR_LIGHT )
			l = (*lightIter)->getVector();
		else if( id == POS_LIGHT ){
			l = (*lightIter)->getVector() - p;
			l = l / l.length();					// this is somehow 0
		}

		ray_t shadowRay;
		hit_t intersection;

		if( l.length() <= gmEPSILON ){
			cerr << "the l vector" << endl;
		}

		// store the rays direction, l, and origin, p.
		shadowRay.set_origin(p);
		shadowRay.set_dir(l);

		// if the surface is not occluded from the light by
		//  another surface, calc the diffuse/specular
		if( ! (checkIntersections(
						shadowRay,
						gmEPSILON,
						std::numeric_limits<double>::infinity(),
						intersection,
						time,
						true)))
		{
			/////////////
			// DIFFUSE //
			/////////////
			
			// the diffuse component
			gmVector3 diffuse = gmVector3( 0.0, 0.0, 0.0 );

			// Apply tone shading to the diffuse 
			//  if this material uses it.
			if( surfaceMat->hasToneShadingOn() ){

				double b = surfaceMat->getToneParam_b(),
					   y = surfaceMat->getToneParam_y(),
					   alpha = surfaceMat->getToneParam_alpha(),
					   beta = surfaceMat->getToneParam_beta();

				// calculate the tone shading components
				gmVector3 kBlue = gmVector3( 0.0, 0.0, b ),
						  kYellow = gmVector3( y, y, 0.0 );

				gmVector3 kCool = gmVector3( kBlue[0] + matColor[0]*alpha,
						kBlue[1] + matColor[1]*alpha,
						kBlue[2] + matColor[2]*alpha );

				gmVector3 kWarm = gmVector3( kYellow[0] + matColor[0]*beta,
						kYellow[1] + matColor[1]*beta,
						kYellow[2] + matColor[2]*beta );

				double LdotN = dot( l, normal );
				double temp = ( 1+LdotN ) / 2;

				diffuse = ( kWarm * temp ) + ( kCool * (1-temp) );
			}
			// Otherwise, use the regular phong diffuse.
			else{
				// I used some temps to separate 
				//  the mutliplying for cleanliness
				gmVector3 diff1;	//temp
				double diff2;		//temp

				diff1[0] = matColor[0] * lightCol[0];
				diff1[1] = matColor[1] * lightCol[1];
				diff1[2] = matColor[2] * lightCol[2];

				diff2 = dot(normal, l);

				// calc the complete diffuse term
				diffuse[0] = diff1[0] * max( (double)0, diff2 );
				diffuse[1] = diff1[1] * max( (double)0, diff2 );
				diffuse[2] = diff1[2] * max( (double)0, diff2 );
			}

			//////////////
			// SPECULAR //
			//////////////

			gmVector3 specular,
					  r;
			double rTemp, specTemp;	//temp
			int phongExp = surfaceMat->getPhongExponent();
						
			// temps
			rTemp = dot(normal, l);
			rTemp = 2 * rTemp;
			r = normal * rTemp;
			r = r - l;
						
			// A HARD LESSON WAS LEARNED HERE ON FEB 3. ///////////////
			//  "I have to normalize"
			specTemp = dot( eye.normalize(), r.normalize() );	
			///////////////////////////////////////////////////////////
			
			// make sure its >= 0
			specTemp = max((double)0.0, specTemp);
			// apply phong's exponent
			specTemp = pow( specTemp, phongExp );

			// calculate the specular light
			specular = lightCol * specTemp;
						
			// add the two colors into the final
			phongColor += diffuse + specular;
		}
	}

	return phongColor;
}


gmVector3 Scene::calcReflectColor(material* surfaceMat, ray_t& ray, hit_t& hit, 
		int recDepth, int numRaysToCast, double time){


	gmVector3 avgReflectCol,
			  normal = hit.getNormal(),
			  dirNorm = ray.get_dir_norm(),
			  // calculate the next ray direction
			  r = dirNorm - 2 * ( dot(dirNorm, normal) ) * normal;

	int rayModifier = 0;

	// optimize the glossy step so it doesn't supersample
	//  unnecessarily.
	if( surfaceMat->getGloss() <= 0 )
			numRaysToCast = 1;

	// this loop super-samples the reflective rays cast
	for( int count=0; count < numRaysToCast; count++){

		// the jittered ray direction (if the surface is glossy)
		gmVector3 rPrime;

		double glossIndex = surfaceMat->getGloss();

		// if this object has a glossy material
		if( glossIndex > 0 ){

			// calculate the 
			gmVector3 w = r / r.length();

			// get u and v
			gmVector3 t = w;

			// determine the lowest magnitude coord in t and set it to "1"
			if( t[0] < t[1] ){

				if( t[0] < t[2] )
					t[0] = 1.0;
				else
					t[2] = 1.0;
			}
			else if( t[2] < t[1] ){

				if( t[2] < t[0] )
					t[2] = 1.0;
				else
					t[0] = 1.0;
			}
			else
				t[1] = 1;

			// calculate the orthonormal vectors
			gmVector3 theCross = cross(t,w);
			gmVector3 u = theCross / theCross.length();
			gmVector3 v = cross( w, u );

			// jitter t of the reflection ray
			double xRand, 
				   yRand;

			do { // Rejection sampling
				xRand = randDouble();
				yRand = randDouble();
			}while( ((xRand*xRand) + (yRand*yRand)) > 1 );

			// calculate the coefficient to the u and v vectors
			double uCoeff = -(glossIndex/2) + (xRand*glossIndex);
			double vCoeff = -(glossIndex/2) + (yRand*glossIndex);

			//cerr << "inhere" << endl;
			rPrime = r + u*uCoeff + v*vCoeff;
		}
		else{
			//cerr << "gothere" << endl;
			rPrime = r;
		}

		if( rPrime.length() <= gmEPSILON ){
			cerr << "rPrime" << endl;
		}

		ray_t nextRay;
		// initialize the next ray to be cast
		nextRay.set_origin(ray.get_p());
		nextRay.set_dir(rPrime);

		// using "tempColor" may be unnecessary, but it keeps things clear
		gmVector3 tempColor = castRay(nextRay, 
				gmEPSILON, std::numeric_limits<double>::infinity(),
				time, recDepth);

		// this 'if' makes it so that the bg color is not reflected
		//  by the reflected surfaces in the scene.
		if( tempColor == scene.getBGColor() ){
			// modify the ray count in the division of the avg color
			//  so that this ray is not included
			rayModifier++;
			// add 0 color to the average color
			tempColor = gmVector3();
		}

		avgReflectCol += tempColor;
	}

	avgReflectCol = avgReflectCol/max( 1 , (numRaysToCast - rayModifier) );
	
	return avgReflectCol;
}


bool Scene::refractRay(gmVector3 d, gmVector3 n, double nIndex, double ntIndex,
		ray_t& t, double dDOTn){

	// ray direction
	gmVector3 dir;

	// first, calculate the square root whether or not there is refraction
	double underSqrt = ( 1 - (( 1/pow(ntIndex,2) )*pow(nIndex,2)*( 1-pow(dDOTn,2))));

	if( underSqrt < 0 )
		return false;
	else{

		underSqrt = sqrt(underSqrt);

		// this calculates the direction of the transmitted ray
		dir = (1/ntIndex)*(nIndex*(d-n*dDOTn)) - n*underSqrt;
		//dir.normalize();

		t.set_dir(dir);

		return true;
	}
}


gmVector3 Scene::calcRefractColor(material* surfaceMat, ray_t& ray, hit_t& hit, 
		gmVector3 reflectCol, int recDepth, int numRaysToCast, double time){

	ray_t tRay;					// the transmitted ray
	gmVector3 transmitCol;		// the color returned by the transmitted ray
	double c = 1.0;
	double kr = 1.0,			// extinction coefficients
		   kg = 1.0,
		   kb = 1.0;			// the good
	double nt = 1.0,
		   n = 1.0;

	// pre-calc dot(dir, surfaceNormal)
	double dDOTn = dot( ray.get_dir_norm(), hit.getNormal());

	// catches the return from refractRay
	bool canRefract = false;

	// if the dot product of ray.dir ( d ) and the surface normal ( n )
	//  is < 0 then the ray has arrived from outside of the sphere
	if( dDOTn < 0 ){
		
		n = 1.0;//ray.get_refr_index();
		nt = surfaceMat->getRefractIndex();
		//surfaceMat->setLastRefrIndex( n );


		// transmit a bent ray from the index of the 
		//  environment(1.0) into the object
		canRefract = refractRay( ray.get_dir_norm(), hit.getNormal(), 
				n, nt, tRay, dDOTn);
		
		// calculate c
		c = dot( -ray.get_dir_norm(), hit.getNormal() );
	}
	else{
		// get the refraction extinction of the surface's material
		gmVector3 extinction = surfaceMat->getRefractExtinction();

		// calculate the final extinction coeffs
		kr = exp( ( -1 * extinction[0] * hit.getT() ) );
		kg = exp( ( -1 * extinction[1] * hit.getT() ) );
		kb = exp( ( -1 * extinction[2] * hit.getT() ) );

		n = surfaceMat->getRefractIndex();
		nt = 1.0;//ray.get_refr_index();
		//surfaceMat->setLastRefrIndex( n );

		canRefract = refractRay( ray.get_dir_norm(), -hit.getNormal(), 
				n, nt, tRay, dDOTn);

		if( testPixel ){
			cerr << endl << "tRay.get_dir_norm(): " << tRay.get_dir_norm() << endl
				<< "n: " << n << endl << "nt: " << nt << endl;
		}
	
		// if the transmit color is != NULL
		if( canRefract ){
			c = dot( tRay.get_dir_norm(), hit.getNormal() );
		}
		else{
			// total internal reflection //
			// attenuate the pre-established finalColor
			transmitCol[0] = kr*reflectCol[0];
			transmitCol[1] = kg*reflectCol[1];
			transmitCol[2] = kb*reflectCol[2];
			
			//return this color, at this point
			return transmitCol;
		}
	}

	// super-sample the rays(when blur > 0
	gmVector3 avgRefractCol = gmVector3();

	if(surfaceMat->getBlur() <= 0)
		numRaysToCast = 1;

	// this loop super-samples the reflective rays cast
	for( int count=0; count < numRaysToCast; count++){

		gmVector3 tPrime,
				  t = tRay.get_dir_norm();
		double blurIndex = surfaceMat->getBlur();

		tPrime = t;

		// if this object has a blurry material
		if( blurIndex > 0 ){

			// calculate the 
			gmVector3 w = t / t.length();

			// get u and v
			gmVector3 r = w;

			// determine the lowest magnitude coord in t and set it to "1"
			if( r[0] < r[1] ){

				if( r[0] < r[2] )
					r[0] = 1.0;
				else
					r[2] = 1.0;
			}
			else if( r[2] < r[1] ){

				if( r[2] < r[0] )
					r[2] = 1.0;
				else
					r[0] = 1.0;
			}
			else
				r[1] = 1;

			// calculate the orthonormal vectors
			gmVector3 crossProd = cross(r,w);
			gmVector3 u = crossProd / gmVector3(crossProd).length();
			gmVector3 v = cross( w, u );

			// jitter t of the reflection ray
			double xRand, 
				   yRand;

			do { // Rejection sampling
				xRand = randDouble();
				yRand = randDouble();
			}while( ((xRand*xRand) + (yRand*yRand)) > 1 );

			// calculate the coefficient to the u and v vectors
			double uCoeff = -(blurIndex/2) + (xRand*blurIndex);
			double vCoeff = -(blurIndex/2) + (yRand*blurIndex);

			tPrime = t + u*uCoeff + v*vCoeff;
		}
		//else //tRay.dir is already set properly

		tRay.set_dir( tPrime );
		tRay.set_origin( ray.get_p() );
		//tRay.set_refr_index( nt );

		// cast a ray and get the transmitted color
		transmitCol = castRay(tRay, 
				gmEPSILON, std::numeric_limits<double>::infinity(),
				time, recDepth);

		// calculate the reflectance
		double r0 = pow( (nt - 1), 2 ) / pow( (nt + 1), 2 );	
		double R = r0 + ( 1 - r0 ) * pow( ( 1 - c ) , 5 );

		// the final, attenuated, refracted color
		gmVector3 refractCol;
		
		// this 'if' makes it so that the bg color is not reflected
		//  by the reflected surfaces in the scene.
		//if( transmitCol == scene.getBGColor() )
		//	transmitCol = reflectCol;

		// actually attenuating the ray color produced by all of the above code
		refractCol[0] = kr * ( (R * reflectCol[0]) + ((1 - R) * transmitCol[0]));
		refractCol[1] = kg * ( (R * reflectCol[1]) + ((1 - R) * transmitCol[1]));
		refractCol[2] = kb * ( (R * reflectCol[2]) + ((1 - R) * transmitCol[2]));

		avgRefractCol += refractCol;
	}
	
	return avgRefractCol/numRaysToCast;
}

bool Scene::calcEdge( surface_t* surface, ray_t& ray, hit_t& hit ){

	// get the edge thickness
	material* surfaceMat = surface->getMaterial();
	double e = surfaceMat->getEdgeThickness();

	if( ! strcmp( surface->name(), "sphere" ) ){
		// determine if the angle between the normal at the
		//  hit point of the surface and the eye direction
		//  is [ -e, e ]

		// calc the dot product of the eye and surface normal at hit.t
		double EdotN = dot( ray.get_dir(), hit.getNormal() );
	
		// if the dot product is within the threshold
		//  of the edge thickness, ...
		if( (EdotN > -e) && (EdotN <= e) ){
			// ... return the edge color.
			return true;
		}
		else
			return false;
	}
	/*else if( ! strcmp( surface->name(), "polygon" ) ){
		gmVector3 p = ray.get_p();
		
		plane dPlane = ((polygon*)surface)->getFace(hit.getHitFaceIndex());

		// iterate over the vertices, 
		//  until we determine an edge or
		//  cannot check any more vertices.
		for(int v=1; v <= dPlane.getNumVertices(); v++ ){

			gmVector4 vert0,
					  vert1;

			if( v == dPlane.getNumVertices() ){
				vert0 = dPlane.getVertex( 0 );
				vert1 = dPlane.getVertex( v-1 );
			}
			else{
				vert0 = dPlane.getVertex( v );
				vert1 = dPlane.getVertex( v-1 );
			}

			gmVector3 v0 = FourToThree( &vert0 ),
					  v1 = FourToThree( &vert1 );

			gmVector3 edge = v1 - v0;

			gmVector3 normal = hit.getNormal();		// get the normal of the surface hit

			int u, v;
			
			// determine which axis plane to which to flatten the surface
			if( ( gmAbs(edge[2]) > gmAbs(edge[0]) ) && 
					( gmAbs(edge[2]) > gmAbs(edge[1]) ) )
			{
				u = 0;
				v = 1;
			}
			else if( gmAbs(edge[1]) > gmAbs(edge[0]) ){
				u = 0;
				v = 2;
			}
			else{
				u = 1;
				v = 2;
			}

			// compute the coordinate on the line where the tangent extends to the point
			double t = ( ( (p[u] - v0[u])*(v1[u] - v0[u]) ) + ( (p[v] - v0[v])*(v1[v] - v0[v]) ) );
			t = t / ( v1 - v0 ).length();

			double x_t = v0[u] + t * (v1[u] - v0[u]),
				   y_t = v0[v] + t * (v1[v] - v0[v]);

			// calculate the distance that p is from point (x,y) on the line
			double distance = sqrt( (p[u] - x_t) + (p[v] - y_t) );

			if( distance <= e )
				return true;
			else
				return false;
		}
	}*/

	return false;
}

// calculate the copperplate engraving hatching pattern(s)
gmVector3 Scene::calcHatching(surface_t* surface, ray_t& ray, hit_t& hit,
		gmVector3 finalColor){

//
// This is the formula from the Leister paper
//  It was poorly explained.
//  kappa = Union{0 to 1}( Union( c_0*b_0 , r*c_0 ) ,Union)( c_1*b_1 , r*c_0 ) ) 
//  			+ Sum( d_0 * b_0, d_1 * b_1 ) + r * d_0
//  			+ (h - hb) * hf
////////////////////////////////////////////////////////////////////////////////

	gmVector3 t = ray.get_p();
	material* surfaceMat = surface->getMaterial();
	hatch_set* hatches = surfaceMat->getHatches();

	// data I've stored for readability
	double a0 = hatches->get_a0(),
		   c0 = hatches->get_c0(),
		   d0 = hatches->get_d0(),
		   l = fmod(hit.getT(), a0) / a0;

	// intermed. calcs for kappa
	double LtimesC0 = l * c0,
		   h = (finalColor[0] + finalColor[1] + finalColor[2]) / 3.0,
		   //h = 0.5,
		   thickness = ( h - hatches->get_hb() ) * hatches->get_hf(),
		   LtimesD0 = l * d0;
	
	// the outer union of the kappa calc
	// 	I'll accumulate this over time
	double unionPart = 0.0;
	double theSum = 0.0;

	// loop over the lamina, calculating the hatching
	for( int h=0; h < hatches->numOfLamina(); h++ ){

		// get the h-th lamina
		lamina* lam = hatches->getLamina(h);

		// get the lamina properties
		double a = lam->_a,				// the offset between planes
			   e1 = (*lam->_e)[0],		// e_i,1
			   e2 = (*lam->_e)[1],		// e_i,2
			   e3 = (*lam->_e)[2],		// e_i,3
			   e4 = (*lam->_e)[3],		// e_i,4
			   f = e1*t[0] + e2*t[1] + e3*t[2] + e4;	// the plane equation

		// calculate b_i
		double b;
		if( f >= 0 )
			b = fmod(f, a) / a;
		else // f < 0
			b = a - ( fmod(f, a) / a );

		double innerUnion,
			   DtimesB = lam->_d * b,
			   CtimesB = lam->_c * b;

		// add DtimesB to the sum ( for use in the Kappa calc )
		theSum += DtimesB + thickness + LtimesD0;

		if( LtimesC0 >= 0 )
			innerUnion = max( CtimesB, LtimesC0 );
		else
			innerUnion = min( CtimesB, 1 - LtimesC0 );
	
		if( h > 0 ){
			if( innerUnion >= 0 )
				unionPart = max( unionPart , innerUnion );
			else
				unionPart = min( unionPart , 1 - innerUnion );
		}
		else
			unionPart = innerUnion;
	}

	double kappa = unionPart + theSum;// + LtimesD0 + thickness;
	
	if( kappa >= 0.5 )
		return gmVector3( 1, 1, 1 );
	else{
		//return gmVector3( 0, 0, 0 );
		return surfaceMat->getEdgeColor();
	}
}

gmVector3 Scene::calcPointColor(surface_t* surface, ray_t& ray, hit_t& hit, 
		gmVector3 eye, int recDepth, double time){

	// get the surface's material
	material* surfaceMat = surface->getMaterial();
 
	// this is the structure that will be ultimately returned
	gmVector3 finalColor = gmVector3(.5,.5,.5);

	///////////////
	// Edge Drawing 
	////////////////
	
	// if this object has edges, ...
	if( surfaceMat->hasEdges()  ){
		if( calcEdge( surface, ray, hit ) ){
			return surfaceMat->getEdgeColor();
		}
	}
	
	/////////////
	// Tex Mapping
	///////////////

	gmVector3 matColor = gmVector3();
	// if the material has a texture map,
	if( surfaceMat->isTextureMapped() ){
		// get the color from the texture map
		matColor = calcTexColor( surface, ray, hit );
	}
	else{
		matColor  = surfaceMat->getColor();
	}

	// UPDATE the final color
	finalColor = matColor;

	//////////////
	// Phong Shading
	/////////////////

	if( surfaceMat->hasShadingOn() ){
		// calculate the phong color
		finalColor = calcPhongColor(surfaceMat, ray, hit, finalColor, time, eye);
	}
	else{
		// return the mat color
		return finalColor;
	}

	// if the recursion is shallow enough, 
	//  enable super-sampling of the reflection ray.
	unsigned numRaysToCast = 1;
	if( recDepth < 2 ){
		numRaysToCast = 5;
	}

	// increment the recursion depth
	recDepth++;

	////////////
	// Reflection
	//////////////

	gmVector3 reflectColor = gmVector3();
	if( surfaceMat->isReflective() && recDepth < 10 ){

		reflectColor = calcReflectColor(surfaceMat, ray, hit, 
				recDepth, numRaysToCast, time);

		finalColor[0] = finalColor[0] + ( ( 1-finalColor[0] )*reflectColor[0]*surfaceMat->getReflect()[0] );
		finalColor[1] = finalColor[1] + ( ( 1-finalColor[1] )*reflectColor[1]*surfaceMat->getReflect()[1] );
		finalColor[2] = finalColor[2] + ( ( 1-finalColor[2] )*reflectColor[2]*surfaceMat->getReflect()[2] );
	}

	////////////
	// Refraction
	//////////////

	gmVector3 refractColor = gmVector3();
	// if this surface has refractive components (is dielectric)
	if( surfaceMat->isDielectric() && recDepth < 10 ){

		refractColor = calcRefractColor(surfaceMat, ray, hit, reflectColor, 
				recDepth, numRaysToCast, time);

		gmVector3 refrExtinct = surfaceMat->getRefractExtinction();

		finalColor[0] = finalColor[0] + (( 1-finalColor[0])*refractColor[0]);
		finalColor[1] = finalColor[1] + (( 1-finalColor[1])*refractColor[1]);
		finalColor[2] = finalColor[2] + (( 1-finalColor[2])*refractColor[2]);
		//
		//finalColor = refractColor;
	}
	
	//////////////
	// Copperplate Engraved Hatching
	/////////////////////////////////
	if( surfaceMat->hasHatches() ){
		return calcHatching( surface, ray, hit, finalColor );
	}

	return finalColor;
}


