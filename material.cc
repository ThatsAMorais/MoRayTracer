// *************************************************************************
//    file: material.cc
//  author: Alex Morais
// purpose: a class to hold materials of surfaces(primitive, but under
// 			construction)
// created: 1-23-2007
// *************************************************************************

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "material.h"
//#include <>	// an image library

void material::read( std::istream& ins ){
	
	std::string cmd;                    // Buffer to hold each command
	bool   seen_end_tag  = false;  // Stop reading at end of view block
	bool   seen_color = false;     // Must see each command
	bool   seen_ambient = false;
	bool   seen_phong = false;
	bool   seen_extinct = false;
	bool   seen_refr_index = false;

	//// initialize private data ////
	// init the material booleans
	bShadingOn = false;
	bIsDielectric = false;
	bIsReflective = false;
	bIsTextureMapped = false;

	// init the material properties
	ambient = gmVector3();
	color = gmVector3();
	phongExp = 1;
	reflectivity = gmVector3();
	gloss = 0.0;
	blur = 0.0;
	refrExtinction = gmVector3();
	refrIndex = 0.0;
	texMap = NULL;
	///////////
		
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
			if( reflectivity.length() != 0 )
				bIsReflective = true;
		}
		// refractivity extinction
		else if( cmd == "refract_extinct" ){
			// get the extinction coeffs
			gmVector3 coeffs;
			ins >> coeffs;

			// calculate the natural log as an optim. for ..
			//  k_r = exp( -log( a_r ) * t );
      		//  k_g = exp( -log( a_g ) * t );
			//  k_b = exp( -log( a_b ) * t );
			refrExtinction[0] = -log( coeffs[0] );
			refrExtinction[1] = -log( coeffs[1] );
			refrExtinction[2] = -log( coeffs[2] );

			//
			seen_extinct=true;	
		}
		// refractive index
		else if( cmd == "refract_index" ){
			ins >> refrIndex;
			seen_refr_index=true;
		}
		// gloss
		else if( cmd == "gloss" ){ ins >> gloss; }
		//blur
		else if( cmd == "blur" ){ ins >> blur; }
		// texture maps
		else if( cmd == "texture" ){

			////
			// This supports .ppm only
			/////

			// a path var to the texture
			std::string texPath;
			
			// read-in the texture path
			ins >> texPath;

			if( texPath[0] != '\0' ){

				// open the file
				std::ifstream texIn;

				// remove quotation marks from the path
				//  (I guess this means no spaces allowed)
				if(texPath[0] == '\"'){
					texPath = strtok(&texPath[1], "\"");
				}

				texIn.open(texPath.data(), std::ifstream::in);

				// if successful, read-in the width and height of the texture
				////
				if( texIn.is_open() ){

					// skip the "P#" line
					texIn.ignore( std::numeric_limits<int>::max(), '\n' );

					// get the extents of the texture
					texIn >> texWidth;
					texIn >> texHeight;
				
					// indicate a successful texture load
					bIsTextureMapped = true;

					// create a texture structure big enough to hold the image
					texMap = new gmVector3[texWidth*texHeight];

					// read-in the texture map to the array
					for(int v=0; v<texHeight; v++){
						for(int u=0; u<texWidth; u++){

							texIn >> texMap[u + (v * texWidth)];
							texMap[u + (v * texWidth)] = texMap[u + (v * texWidth)] / 255;
							
							// a little output
							//std::cerr << x << " " << y << " " << z << "; ";
							//std::cerr << texMap[u + (v*width)] << "; " << std::endl;
						}
					}
				}
				else
					std::cerr << "The file was not opened" << std::endl;

				// close that shit
				texIn.close();
			}
			//else
			//	std::cerr << "[ texPath[0] == \\0 ]" << std::endl;
		}
	}

	// turn shading on/off depending on if the proper
	//  input values are provided.
	if( seen_ambient && seen_phong )
		bShadingOn = true;

	// determine if this material has the right properties
	//  specified to be a dielectric
	if( seen_extinct && seen_refr_index )
		bIsDielectric = true;
}

gmVector3 material::getColor(double x, double y){

	// map the pixel to the appropriate location
	//  in the texture map
	
	int mappedX= (int)( x * (texWidth-1) ),
		mappedY= (int)( y * (texHeight-1) );
	
	return texMap[mappedX + (mappedY*texWidth)];
}
