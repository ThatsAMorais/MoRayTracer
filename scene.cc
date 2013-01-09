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
			ins >> background;
			seen_bg = true;
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

	if(!seen_a_surface)
		cerr << "Did not see any surfaces in the script." << endl;
}
/////////////////////////////////////////////////

material* Scene::checkIntersections(ray_t& ray, double t0, double t1, hit_t& hit, 
		bool checkOne)
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
		if((*surfaceIter)->intersect(ray, t0, t1, hit)){

			// ---Optimization---
			// for checking a single intersection
			//  when you don't need the closest t
			if(checkOne){
				return (*surfaceIter)->getMaterial();
			}

			//if(hit.getT() < t1){
				t1 = hit.getT();
				nearestSurface = (*surfaceIter);
			//}
		}
		//else
		//	hit.setT(std::numeric_limits<double>::infinity());
	}

	if( nearestSurface )
		return nearestSurface->getMaterial();

	return NULL;
}

bool Scene::checkReflectAndRefract(gmVector3& color, ray_t& thisRay, int recDepth, gmVector3 eye){

	hit_t hit;
	material* hitSurfaceMat;
	gmVector3 hitSurfaceCol,
			  tempCol;

	// determine if the reflection ray hits another suface.. //
	//   if it does, calculate reflection ray and recurse.   //
	if( (hitSurfaceMat =checkIntersections(
				thisRay, 
				gmEPSILON,
				std::numeric_limits<double>::infinity(), 
				hit)) )
	{
		bool reflResult = false;

		ray_t nextRay;	
		if(hitSurfaceMat->getReflect().length() != 0 || recDepth <= 10){
			recDepth++;

			// parameters used to calculate next ray from previous ray
			gmVector3 p = thisRay.get_origin() + hit.getT()*thisRay.get_dir_norm(),
					  normal = hit.getNormal(),
					  dirNorm = thisRay.get_dir_norm();

			// calculate the next ray direction
			gmVector3 r = dirNorm - 2 * ( dot(dirNorm, normal) ) * normal;
		
			// initialize the next ray to be cast
			nextRay.set_origin(p);
			nextRay.set_dir(r);

			// recurse to determine next reflection
			reflResult = checkReflectAndRefract(
					tempCol,
					nextRay,
					recDepth,
					eye);
		}

		//c1 = calc_color(p1)
		//c1 = c1 + (1-c1)*c2*reflectivity1
		//c2 = calc_color(p2)

		// convenience vars for storing the colors
		gmVector3 reflectivity = hitSurfaceMat->getReflect();
		//
		hitSurfaceCol = calcPointColor(hitSurfaceMat, nextRay, hit, eye);
		//
		gmVector3 tempColor;


		if(reflResult){
			tempColor[0] = hitSurfaceCol[0] + ( ( 1-hitSurfaceCol[0] )*tempCol[0]*reflectivity[0] );
			tempColor[1] = hitSurfaceCol[1] + ( ( 1-hitSurfaceCol[1] )*tempCol[1]*reflectivity[1] );
			tempColor[2] = hitSurfaceCol[2] + ( ( 1-hitSurfaceCol[2] )*tempCol[2]*reflectivity[2] );


		}
		else{
			tempColor = hitSurfaceCol;
		}

		color = tempColor;

		return true;
	}
	else
		return false;

	
}

gmVector3 Scene::calcPointColor(material* surfaceMat, ray_t& ray, hit_t& hit, gmVector3 eye){

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

		// shoot a shadow ray and determine if the 
		// light occludes "this" surface.
									
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
			specTemp = max((double)0, specTemp);
			// apply phong's exponent
			specTemp = pow( specTemp, phongExp );

			// calculate the specular light
			specular = lightCol * specTemp;
						
			// add the two colors into the final
			finalColor += diffuse + specular;
		}
	}

	return finalColor;
}
