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
	bool   seen_reflectivity = false;
	
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
		// Read in commands
		else if( cmd == "color" ){			// color
			ins >> color;
			color.clamp(0.0, 1.0);
			seen_color = true;
		}
		else if( cmd == "ambient" ){		// ambient
			ins >> ambient;
			seen_ambient = true;
		}
		else if( cmd == "phong" ){			// phong
			ins >> phongExp;
			seen_phong = true;
		}
		else if( cmd == "reflectivity" ){	// reflectivity
			ins >> reflectivity;
			seen_reflectivity = true;
		}
	}

	// turn shading on/off depending on if the proper
	//  input values are provided.
	if( !( seen_ambient && seen_phong && seen_reflectivity ) )
		bShadingOn = false;
	else
		bShadingOn = true;
}
