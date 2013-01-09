// *************************************************************************
//    file: material.cc
//  author: Alex Morais
// purpose: a class to hold materials of surfaces(primitive, but under
// 			construction)
// created: 1-23-2007
// *************************************************************************

#include <iostream>
#include "material.h"

void material::read( std::istream& ins ){
	
	std::string cmd;                    // Buffer to hold each command
	bool   seen_end_tag  = false;  // Stop reading at end of view block
	bool   seen_color = false;     // Must see each command
	bool   seen_ambient = false;
	bool   seen_phong = false;
	bool   seen_extinct = false;
	bool   seen_refr_index = false;

	// initialize the material booleans
	bShadingOn = false;
	bIsDielectric = false;
	bIsReflective = false;

	// initialize private data
	ambient = gmVector3();
	color = gmVector3();
	phongExp = 1;
	reflectivity = gmVector3();
	gloss = 0.0;
	refrExtinction = gmVector3(1,1,1);
	refrIndex = 0.0;
	
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
		else if( cmd == this->end_tag() ){seen_end_tag = true; 	}
		// Read in commands
		else if( cmd == "color" ){			// color
			ins >> color;
			color.clamp(0.0, 1.0);
			seen_color = true;
		}
		// ambient
		else if( cmd == "ambient" ){
			ins >> ambient;	
			seen_ambient = true; 
		}
		// phong
		else if( cmd == "phong" ){
			ins >> phongExp;	
			seen_phong = true;	
		}
		// reflectivity
		else if( cmd == "reflectivity" ){ 
			ins >> reflectivity;
			bIsReflective = true;
		}
		// refractivity extinction
		else if( cmd == "refract_extinct" ){
			ins >> refrExtinction; 
			seen_extinct=true;	
		}
		// refractive index
		else if( cmd == "refract_index" ){ 
			ins >> refrIndex; 
			seen_refr_index=true;
		}
		// gloss
		else if( cmd == "gloss" ){ ins >> gloss; }
	}

	// turn shading on/off depending on if the proper
	//  input values are provided.
	if( seen_ambient && seen_phong )
		bShadingOn = true;

	if( reflectivity.length() )
		bIsReflective = true;

	// determine if this material has the right properties
	//  specified to be a dielectric
	if( seen_extinct && seen_refr_index )
		bIsDielectric = true;
}
