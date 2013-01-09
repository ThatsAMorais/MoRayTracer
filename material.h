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
#include "hatch_set.h"

class material{
	public:
		material();
		~material();
		char* begin_tag() { return "begin_material"; }
		char* end_tag() { return "end_material"; }

		// .ray script parsing function
		void read( std::istream& ins );
		gmVector3 getColor( void ){ return color; }
	
		// Tex Mapping
		gmVector3 getColor(double x, double y);

		// Phong Shading
		void setColor( gmVector3 newCol ){ color = newCol; }
		gmVector3 getAmbient( void ){ return ambient; }
		int getPhongExponent( void ){ return phongExp; }
		
		// Reflection / Glossy Reflections
		gmVector3 getReflect( void ){ return reflectivity; }	
		double getGloss( void ){ return gloss; }

		// Refraction / Blurry Refractions
		gmVector3 getRefractExtinction( void ){ return refrExtinction; }
		double getRefractIndex( void ){ return refrIndex; }
		//double getLastRefrIndex( void ){ return lastRefrIndex; }
		//void setLastRefrIndex( double n ){ lastRefrIndex = n; }
		double getBlur( void ){ return blur; }
		
		// Tone Shading
		double getToneParam_b( void ){ return toneParam_b; }
		double getToneParam_y( void ){ return toneParam_y; }
		double getToneParam_alpha( void ){ return toneParam_alpha; }
		double getToneParam_beta( void ){ return toneParam_beta; }

		// Silhouette and Contour edges
		gmVector3 getEdgeColor( void ){ return edgeColor; }
		double getEdgeThickness( void ){ return edgeWidth; }

		// Hatching lines
		hatch_set* getHatches( void ){ return &hatches; }
		
		// Boolean accesssors
		bool hasShadingOn( void ){ return bShadingOn; }	// uses ambient & diffuse
		bool hasToneShadingOn( void ){ return bToneShadingOn; }	// uses tone shading
		bool isTextureMapped( void ){ return bIsTextureMapped; }// is texture mapped
		bool isReflective( void ){ return bIsReflective; }		// is reflective
		bool isDielectric( void ){ return bIsDielectric; }		// is refractive
		bool hasEdges( void ){ return bHasEdges; }			// has colored edges
		bool hasHatches( void ){ return bHasHatches; }		// has hatching lines
	
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

		// tone shading params
		double toneParam_b,		// the blue amount
			   toneParam_y,		// the yellow amount
			   toneParam_alpha,	// the cool factor
			   toneParam_beta;	// the warm factor

		// hatching pattern params
		hatch_set hatches;
		
		// edge-related
		gmVector3 edgeColor;	// the edge color of this object
		double edgeWidth;		// the edge width

		bool bShadingOn;		// this mat has shadows
		bool bToneShadingOn;	// this mat is tone shaded
		bool bIsReflective;		// this mat is reflective
		bool bIsDielectric;		// this mat refracts rays
		bool bIsTextureMapped;	// this mat has a texture map
		bool bHasEdges;			// this object should be drawn with edges
		bool bHasHatches;		// this object is a copperplate engraving
};

#endif


