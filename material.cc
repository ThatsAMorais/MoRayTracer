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

material::material( void ){

	bShadingOn = false;
	bToneShadingOn = false;
	bIsDielectric = false;
	bIsReflective = false;
	bIsTextureMapped = false;
	bHasEdges = false;
	bHasHatches = false;

	// init the material properties
	// mat color
	color = gmVector3();
	// phong shading
	phongExp = 1;
	ambient = gmVector3();
	// edges
	edgeColor = gmVector3(0.0, 0.0, 0.0);
	edgeWidth = 0.3;
	// reflection/gloss
	reflectivity = gmVector3();
	gloss = 0.0;
	// refraction/blue
	blur = 0.0;
	refrExtinction = gmVector3();
	refrIndex = 0.0;
	// tex mapping
	texMap = NULL;
	// tone shading
	toneParam_b = 0.5;
	toneParam_y = 0.5;
	toneParam_alpha = 0.25;
	toneParam_beta = 0.25;
}

material::~material( void ){}

void material::read( std::istream& ins ){
	
	std::string cmd;               // Buffer to hold each command
	bool   seen_end_tag  = false;  // Stop reading at end of view block
	bool   seen_color = false;     // Must see each of these commands for
								   // all of the features to work
	// Phong bools
	bool   seen_ambient = false;
	bool   seen_phong = false;

	// refraction bools
	bool   seen_extinct = false;
	bool   seen_refr_index = false;

	// tone shading bools
	bool	seen_b = false;
	bool	seen_y = false;
	bool	seen_alpha = false;
	bool	seen_beta = false;

	//// initialize private data ////
	// init the material booleans
	bShadingOn = false;
	bToneShadingOn = false;
	bIsDielectric = false;
	bIsReflective = false;
	bIsTextureMapped = false;
	bHasEdges = false;
	bHasHatches = false;

	// init the material properties
	// mat color
	color = gmVector3();
	// phong shading
	phongExp = 1;
	ambient = gmVector3();
	// edges
	edgeColor = gmVector3(0.0, 0.0, 0.0);
	edgeWidth = 0.3;
	// reflection/gloss
	reflectivity = gmVector3();
	gloss = 0.0;
	// refraction/blue
	blur = 0.0;
	refrExtinction = gmVector3();
	refrIndex = 0.0;
	// tex mapping
	texMap = NULL;
	// tone shading
	toneParam_b = 0.5;
	toneParam_y = 0.5;
	toneParam_alpha = 0.25;
	toneParam_beta = 0.25;
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

		// Edges ////
		else if( cmd == "edge_width" ){
			// edge width
			ins >> edgeWidth;

			if( edgeWidth > 0 ){
				//edgeWidth += 1;
				bHasEdges = true;
			}
		}
		else if( cmd == "edge_color" ){
			// edge color
			// get the color
			ins >> edgeColor;
			// set the edge bool
			bHasEdges = true;
		}
		////

		// Copperplate Engraving ////
		else if( cmd == "begin_hatch" ){
			if( hatches.read( ins ) )
				bHasHatches = true;
		}
		////

		// Tone shading params ////
		else if( cmd == "b" ){
			ins >> toneParam_b;
			seen_b = true;
		}
		else if( cmd == "y" ){
			ins >> toneParam_y;
			seen_y = true;
		}
		else if( cmd == "alpha" ){
			ins >> toneParam_alpha;
			seen_alpha = true;
		}
		else if( cmd == "beta" ){
			ins >> toneParam_beta;
			seen_beta = true;
		}
		////

		// Reflectivity ////
		else if( cmd == "reflectivity" ){ 
			ins >> reflectivity;
			if( reflectivity.length() != 0 )
				bIsReflective = true;
		}
		else if( cmd == "gloss" ){ ins >> gloss; }
		////

		// Refractivity ////
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
		else if( cmd == "refract_index" ){
			ins >> refrIndex;
			seen_refr_index=true;
		}
		else if( cmd == "blur" ){ ins >> blur; }
		////

		// Texture Maps ////
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
		}
	}

	//  turn features on depending on if the proper	 //
	//   input values are provided.					 //
	
	// Use Phong Shading?
	if( seen_ambient && seen_phong )
		bShadingOn = true;

	// Use Tone Shading for the diffuse component?
	if( seen_b && seen_y && seen_alpha && seen_beta )
		bToneShadingOn = true;

	// Is this material Refractive?
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
