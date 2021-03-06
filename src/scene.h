// *************************************************************************
//    file: scene.h
//  author: Alex Morais
// purpose: Scene class to store all objects in the scene and lights
// created: 1-24-2007
// *************************************************************************

#ifndef SCENE_H
#define SCENE_H

#include <iostream>
#include <list>
#include <gm.h>
#include "surfaces.h"

using namespace std;

#define DIR_LIGHT 2
#define POS_LIGHT 1

class Light{
	public:
		Light( );
		~Light();
		//void setVector( gmVector3 vec );
		gmVector3 getVector( void );
		void setColor( gmVector3 col );
		gmVector3 getColor( void );
		int getIdentity( void ){ return identity; }

		void read( std::istream& ins );
		char* end_tag( void ){ return "end_light"; }

	protected:
		gmVector3 position;
		gmVector3 direction;
		gmVector3 color;

		int identity;
};

typedef list<surface_t*> surfaceList;
typedef list<Light*> lightList;

class Scene{
	public:
		Scene();
		~Scene();

		// ray parser function
		void read( std::istream& ins );
		//friend std::ostream& operator << ( std::ostream& os, const view_t& );
		char* begin_tag() { return "begin_scene"; }
		char* end_tag()   { return "end_scene"; }
		gmVector3 getBGColor( void ){ return currentBG; }
		//gmVector3 getBGColor1( void ){ return background1; }
		//gmVector3 getBGColor2( void ){ return background2; }
		void calcNextBGColor( int y, int numYPixels );
		bool getHasTwoColors( void ){ return hasTwoBGColors; }

		//
		material* checkIntersections(ray_t& ray, double t0, double t1, 
				hit_t& hit, double time, bool checkOne=false);

		//
		gmVector3 calcPointColor(material* surfaceMat, ray_t& ray, 
				hit_t& hit, gmVector3 eye, int recDepth, double time);

		//
		surfaceList* getSurfaceList() { return &surfacesInScene; }
		lightList* getLightList() { return &lightsInScene; }

	private:
		list<surface_t*> surfacesInScene;
		list<Light*> lightsInScene;

		gmVector3 background1;
		gmVector3 background2;
		gmVector3 currentBG;

		bool hasTwoBGColors;
};



#endif
