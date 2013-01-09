//*****************************************************************************
//    file: mray.cc
// created: 1-29-2005
//  author: J. Edward Swan II
// purpose: Highest level ray tracer routines.
//*****************************************************************************
// edited by: Alex Morais
// 		 for: Adv. Computer Graphics
// 		 	: Assignment #1 - Build a raytracer...
//*****************************************************************************

#include <iostream>
#include <math.h>
#include <gm.h>

using namespace std;

//#include "functions.h"
#include "view.h"
#include "scene.h"

bool testPixel = false;

// Indicate frame construct in input file
#define FRAME_TAG1 "begin_frame"
#define FRAME_TAG2 "end_frame"

static view_t view;       // Hold the view
static Scene scene;

/*****************************************************************************/
static void frame_read( istream& ins )
{
    string cmd;    // Buffer to hold each command

    // Loop and read until we reach the end of the frame construct
    while( !ins.eof() ) {
		ins >> cmd;
		
		// Skip a comment line; ignore input until end-of-line
		if( cmd == "#" ) { ins.ignore( numeric_limits<int>::max(), '\n' ); }
		// Skip a beginning frame tag
		else if( cmd == FRAME_TAG1 ) { }
		// Read in a view block
	    else if( cmd == view.begin_tag() )  { view.read( ins ); }
		
		else if( cmd == scene.begin_tag() ) { scene.read(ins); }
	}
}
/*****************************************************************************/
int main( int argc, char* argv[] )
{
    bool       err = false;	// Invocation error?
    int        opt;		// Command line options
    bool       EchoInputLanguage = false;
    bool       ShowPercentCompleteDisplay = true;

    // Parse the user's command line options
    while(( opt = getopt( argc, argv, "ec" )) != EOF ) {
		switch(opt) {
			case 'e': EchoInputLanguage = true; break;
			case 'c': ShowPercentCompleteDisplay = false; break;
			default:  err = true;
		}
    }

    // Usage error
    if(err) {
		cerr << argv[0] << " [-ecr] < infile > outfile" << endl;
		exit( 1 );
    }

	//cerr << "Reading input script" << endl;

    // Read in the data
    frame_read( cin );

	//cerr << "Outputting header information" << endl;

	cout << "P3" << endl;
	cout << view.getNumXPixels() << " " 
		<< view.getNumYPixels() << endl;
	cout << "255" << endl;

    if( EchoInputLanguage ) {
		cerr << "The view:\n" << view << endl;
    }

	/* Loop over the pixels */
	for(unsigned y = 0; y < view.getNumYPixels(); y++){
		for(unsigned x =0; x < view.getNumXPixels(); x++){

			if(x == 47 && y == 73)		
				testPixel = true;
			else
				testPixel = false;

			// declare the ray sent into the scene, 
			//  nearest t, and nearest surface.
			ray_t thisRay;

			//the hit object
			hit_t hit;

			// calc more parameters to satisfy the ray equation
			double ru = view.getVPu() / view.getNumXPixels(),
				   rv = view.getVPv() / view.getNumYPixels(),
				   llu = -(double)0.5 * view.getVPu() + (double)0.5 * ru,
				   llv = -(double)0.5 * view.getVPv() + (double)0.5 * rv, 
				   ur = llu + x * ru,
				   vr = llv + y * rv,
				   wr = -view.getFocalLength();

			double t0 = 0, 	// always 0 (or so it seems)
				   t1 = std::numeric_limits<double>::infinity();  // the farthest point away

			// setup the ray
			thisRay.set_origin(view.getEye());
			thisRay.set_dir(ur*view.getVectorU() + vr*view.getVectorV() + wr*view.getVectorW());

			// the closest surface mat
			material* hitSurfaceMat = NULL;
			//surface_t* nearestSurface = NULL;

			/* loop over the object list of the scene */
			hitSurfaceMat = scene.checkIntersections(thisRay, t0, t1, hit);
			//nearestSurface = scene.checkIntersections(thisRay, t0, t1, hit);

			if( hitSurfaceMat == NULL ){
				cout << (int)(255 * scene.getBGColor()[0]) << " " 
					<< (int)(255 * scene.getBGColor()[1]) << " "
					<< (int)(255 * scene.getBGColor()[2]) << " ";
			}
			// let the fun begin!
			else{
				gmVector3 tempCol,	// this is what returns from the reflection recursion
						  reflectivity = hitSurfaceMat->getReflect(),
						  refractivity = hitSurfaceMat->getRefract(),
						  dirNorm = thisRay.get_dir_norm(),
						  r = dirNorm - 2 * ( dot(dirNorm, hit.getNormal()) ) * hit.getNormal();
				
				// the ray to be cast for reflection/refraction
				ray_t reflRay;
				reflRay.set_origin( thisRay.get_p() );
				reflRay.set_dir( r );

				//
				gmVector3 finalColor = scene.calcPointColor(hitSurfaceMat, thisRay, hit, view.getEye());

				if( reflectivity.length() != 0 || refractivity.length() != 0 ){
					
					bool result = scene.checkReflectAndRefract( 
							tempCol,
							reflRay,
							0,
							view.getEye() );

					if(result){
						finalColor[0] = finalColor[0]+((1-finalColor[0])*tempCol[0]*reflectivity[0]);
						finalColor[1] = finalColor[1]+((1-finalColor[1])*tempCol[1]*reflectivity[1]);
						finalColor[2] = finalColor[2]+((1-finalColor[2])*tempCol[2]*reflectivity[2]);
					}
					else
						finalColor = scene.calcPointColor(hitSurfaceMat, thisRay, hit, view.getEye());

				}
				else
					finalColor = scene.calcPointColor(hitSurfaceMat, thisRay, hit, view.getEye());
				
				// clamp'em
				finalColor.clamp(0.0, 1.0);

				cout << (int)( 255 * finalColor[0] )
					<< " "
					<< (int)( 255 * finalColor[1] ) 
					<< " "
					<< (int)( 255 * finalColor[2] ) 
					<< " ";

			}
		}
	}
	
	return( 0 );
}
