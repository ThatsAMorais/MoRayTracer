// *************************************************************************
//    file: material.h
//  author: Alex Morais
// purpose: a class to hold materials of surfaces
// created: 1-23-2007
// *************************************************************************

#ifndef MATERIAL_H
#define MATERIAL_H

#include <iostream>
#include <cstring>
#include <gmVec3.h>
#include <vector>

class material{
	public:
		char* begin_tag() { return "begin_material"; }
		char* end_tag() { return "end_material"; }

		// .ray script parsing function
		void read( std::istream& ins );
		gmVector3 getColor( void ){ return color; }
		gmVector3 getColor(double x, double y);
		gmVector3 getAmbient( void ){ return ambient; }
		int getPhongExponent( void ){ return phongExp; }
		gmVector3 getReflect( void ){ return reflectivity; }
		gmVector3 getRefractExtinction( void ){ return refrExtinction; }
		double getRefractIndex( void ){ return refrIndex; }
		//double getLastRefrIndex( void ){ return lastRefrIndex; }
		//void setLastRefrIndex( double n ){ lastRefrIndex = n; }
		double getGloss( void ){ return gloss; }
		double getBlur( void ){ return blur; }
	
		// boolean accesssors
		bool hasShadingOn( void ){ return bShadingOn; }
		bool isTextureMapped( void ){ return bIsTextureMapped; }
		bool isReflective( void ){ return bIsReflective; }
		bool isDielectric( void ){ return bIsDielectric; }
	
	private:
		// the color vector
		gmVector3 color;		// the color of some surface
		gmVector3 ambient;  	// ambient light component
		int phongExp;			// k_spec
		gmVector3 reflectivity;	// self-explanatory
		double gloss;			// this is to generate perturbed samples
		gmVector3 refrExtinction; // the extinction rate of refr color
		double refrIndex;		// the index of refraction
		//double lastRefrIndex;
		double blur;			// blur factor for refraction rays
		gmVector3* texMap;		// the texture map color at points
		int texHeight;			// the height of the texture map
		int texWidth;			// the width of the texture map

		
		bool bShadingOn;		// this mat has shadows
		bool bIsReflective;		// this mat is reflective
		bool bIsDielectric;		// this mat refracts rays
		bool bIsTextureMapped;	// this mat has a texture map
};

#endif


