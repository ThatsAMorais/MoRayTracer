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
		if((*surfaceIter)->intersect(ray, t0, t1, hit, time)){

			//cerr << "we got a hit" << endl;

			// ---Optimization---
			// for checking a single intersection
			//  when you don't need the closest t
			if(checkOne){
				return (*surfaceIter);//->getMaterial();
			}

			t1 = hit.getT();
			nearestSurface = (*surfaceIter);
		}
	}

	if( nearestSurface )
		return nearestSurface;//->getMaterial();

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
						
			// I used some temps to separate 
			//  the mutliplying for cleanliness
			gmVector3 diffuse,
					  diff1;	//temp
			double diff2;		//temp

			diff1[0] = matColor[0] * lightCol[0];
			diff1[1] = matColor[1] * lightCol[1];
			diff1[2] = matColor[2] * lightCol[2];

			diff2 = dot(normal, l);

			// calc the complete diffuse term
			diffuse[0] = diff1[0] * max( (double)0, diff2 );
			diffuse[1] = diff1[1] * max( (double)0, diff2 );
			diffuse[2] = diff1[2] * max( (double)0, diff2 );

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
		gmVector3 phongColor, int recDepth, int numRaysToCast, double time){


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
	
	gmVector3 reflectCol;
	reflectCol[0] = phongColor[0] + ( ( 1-phongColor[0] )*avgReflectCol[0]*surfaceMat->getReflect()[0] );
	reflectCol[1] = phongColor[1] + ( ( 1-phongColor[1] )*avgReflectCol[1]*surfaceMat->getReflect()[1] );
	reflectCol[2] = phongColor[2] + ( ( 1-phongColor[2] )*avgReflectCol[2]*surfaceMat->getReflect()[2] );

	return reflectCol;

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
		gmVector3 currentCol, int recDepth, int numRaysToCast, double time){

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
			transmitCol[0] = kr*currentCol[0];
			transmitCol[1] = kg*currentCol[1];
			transmitCol[2] = kb*currentCol[2];
			
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
			//tPrime.normalize();

			//tRay.set_dir(tPrime);
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
		//	transmitCol = currentCol;

		// actually attenuating the ray color produced by all of the above code
		refractCol[0] = kr * ( (R * currentCol[0]) + ((1 - R) * transmitCol[0]));
		refractCol[1] = kg * ( (R * currentCol[1]) + ((1 - R) * transmitCol[1]));
		refractCol[2] = kb * ( (R * currentCol[2]) + ((1 - R) * transmitCol[2]));

		avgRefractCol += refractCol;
	}
	
	return avgRefractCol/numRaysToCast;
}


gmVector3 Scene::calcPointColor(surface_t* surface, ray_t& ray, hit_t& hit, 
		gmVector3 eye, int recDepth, double time){

	// get the surface's material
	material* surfaceMat = surface->getMaterial();
 
	// this is the structure that will be ultimately returned
	gmVector3 finalColor = gmVector3(.5,.5,.5);
	// if the material has a texture map,
	if( surfaceMat->isTextureMapped() ){
		// get the color from the texture map
		finalColor = calcTexColor( surface, ray, hit );
	}
	else{
		finalColor  = surfaceMat->getColor();
	}

	// calculate the phong color
	if(surfaceMat->hasShadingOn())
		finalColor = calcPhongColor(surfaceMat, ray, hit, finalColor, time, eye);
	else
		return finalColor;

	// if the recursion is shallow enough, 
	//  enable super-sampling of the reflection ray.
	unsigned numRaysToCast = 1;
	if( recDepth < 2 ){
		numRaysToCast = 5;
	}

	// increment the recursion depth
	recDepth++;

	//////////////
	// Reflection
	///////////////

	if( surfaceMat->isReflective() && recDepth < 10 ){

		finalColor += calcReflectColor(surfaceMat, ray, hit, finalColor, 
				recDepth, numRaysToCast, time);
	}

	//////////////
	// Refraction
	///////////////

	// if this surface has refractive components (is dielectric)
	if( surfaceMat->isDielectric() && recDepth < 10 ){

		finalColor = calcRefractColor(surfaceMat, ray, hit, finalColor, 
				recDepth, numRaysToCast, time);
	}

	return finalColor;
}


