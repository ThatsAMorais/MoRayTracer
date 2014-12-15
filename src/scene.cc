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

material* Scene::checkIntersections(ray_t& ray, double t0, double t1, hit_t& hit, 
		double time, bool checkOne)
{	
	//if(!checkOne)
		//cerr << "checking a surface" << endl;

	list<surface_t*>::iterator surfaceIter;
	surface_t* nearestSurface = NULL;

	/* For every object in scene */
	for(surfaceIter = surfacesInScene.begin(); 
			surfaceIter != surfacesInScene.end();
			surfaceIter++)
	{
		/* If ray intersects this object { */
		if((*surfaceIter)->intersect(ray, t0, t1, hit, time)){

			// ---Optimization---
			// for checking a single intersection
			//  when you don't need the closest t
			if(checkOne){
				return (*surfaceIter)->getMaterial();
			}

			t1 = hit.getT();
			nearestSurface = (*surfaceIter);
		}
		//else
		//	hit.setT(std::numeric_limits<double>::infinity());
	}

	if( nearestSurface )
		return nearestSurface->getMaterial();

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

gmVector3 Scene::calcPointColor(material* surfaceMat, ray_t& ray, hit_t& hit, 
		gmVector3 eye, int recDepth, double time){

	// the mat of the surface that the ray intersected
	
	gmVector3 matColor = surfaceMat->getColor();

	/////////////
	// AMBIENT //
	/////////////
	
	if(!surfaceMat->hasShadingOn())
		return surfaceMat->getColor();

	// ambient term
	gmVector3 ambient;
	ambient[0] = ( surfaceMat->getAmbient()[0] * matColor[0] );
	ambient[1] = ( surfaceMat->getAmbient()[1] * matColor[1] );
	ambient[2] = ( surfaceMat->getAmbient()[2] * matColor[2] );

	// Add the ambient light to the final color.
	//  (Obligatory)
	gmVector3 finalColor = ambient;
	/////////////

	// Next we iterate over the lights 
			
	// light list iterator ...
	list<Light*>::iterator lightIter;

	// p => from the ray equation
	gmVector3 rt = ray.get_p();
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
			l = (*lightIter)->getVector() - rt;
			l = l / l.length();					// this is somehow 0
		}

		ray_t shadowRay;
		hit_t intersection;

		// store the rays direction, l, and origin, p.
		shadowRay.set_origin(rt);
		shadowRay.set_dir(l);

		// if the surface is not occluded from the light by
		//  another surface, calc the diffuse/specular
		if( ! (checkIntersections(
						shadowRay,
						gmEPSILON,
						std::numeric_limits<double>::infinity(),
						intersection,
						time,
						false)))
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
			specTemp = max((double)0, specTemp);
			// apply phong's exponent
			specTemp = pow( specTemp, phongExp );

			// calculate the specular light
			specular = lightCol * specTemp;
						
			// add the two colors into the final
			finalColor += diffuse + specular;
		}
	}


	// if material is reflective, cast reflection rays
	ray_t nextRay;	
	if(surfaceMat->getReflect().length() != 0 || recDepth <= 10){

		recDepth++;

		// parameters used to calculate next ray from previous ray
		gmVector3 p = ray.get_origin() + hit.getT()*ray.get_dir_norm(),
				  normal = hit.getNormal(),
				  dirNorm = ray.get_dir_norm();

		// calculate the next ray direction
		gmVector3 r = dirNorm - 2 * ( dot(dirNorm, normal) ) * normal;
		gmVector3 w;

		gmVector3 rPrime;
		double glossIndex = surfaceMat->getGloss();
		// if this object has a glossy material
		if( glossIndex > 0 ){

			// calculate the 
			w = r / r.length();

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
			gmVector3 u = cross(t,w) / gmVector3(cross(t,w)).length();
			gmVector3 v = cross( w, u );

			double xRand, 
				   yRand;

			do { // Rejection sampling
				xRand = randDouble();
				yRand = randDouble();
			}while( ((xRand*xRand) + (yRand*yRand)) > 1 );


			// calculate the coefficient to the u and v vectors
			double uCoeff = -(glossIndex/2) + (xRand*glossIndex);
			double vCoeff = -(glossIndex/2) + (yRand*glossIndex);

			rPrime = r + u*uCoeff + v*vCoeff;
			rPrime.normalize();
		}
		else
			rPrime = r;

		// initialize the next ray to be cast
		nextRay.set_origin(p);
		nextRay.set_dir(rPrime);

		// cast a ray from this reflective surface to check for a hit
		hit_t hit;
		material* reflectedMat = checkIntersections( nextRay, gmEPSILON,
				std::numeric_limits<double>::infinity(),
				hit, time);

		gmVector3 tempColor;
		// if there was an intersection with an object
		if(reflectedMat){
			tempColor = calcPointColor(reflectedMat, nextRay, hit, eye, recDepth, time);
		}

		// This is an option to reflect the Background color when no objects are hit
		//else{
			//set the reflected color to the background color
		//	tempColor = currentBG;
		//}

		finalColor[0] = finalColor[0] + ( ( 1-finalColor[0] )*tempColor[0]*surfaceMat->getReflect()[0] );
		finalColor[1] = finalColor[1] + ( ( 1-finalColor[1] )*tempColor[1]*surfaceMat->getReflect()[1] );
		finalColor[2] = finalColor[2] + ( ( 1-finalColor[2] )*tempColor[2]*surfaceMat->getReflect()[2] );
	}

	// Do refraction stuff here
	

	return finalColor;
}
