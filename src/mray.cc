//*****************************************************************************
//    file: mray.cc
// created: 1-29-2005
//  author: J. Edward Swan II
// purpose: Highest level ray tracer routines.
//*****************************************************************************
// edited by: Alex Morais
// 		 for: Adv. Computer Graphics
// 		 	: This is a side project to make this program multi-threaded
//*****************************************************************************

#include <iostream>
#include <math.h>
#include <gm.h>

// uh oh.... you know what dis means...
//
#include <pthread.h>			// for pthread_create, pthread_t, etc...
#include <fcntl.h>   	 	// for open
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

//#include "functions.h"
#include "view.h"
#include "scene.h"
#include "functions.h"

// pixel isolating bool
bool testPixel = false;

// Indicate frame construct in input file
#define FRAME_TAG1 "begin_frame"
#define FRAME_TAG2 "end_frame"

static view_t view;       // Hold the view
static Scene scene;
static gmVector3* finalImage;

// a structure to define the thread arguments
class thread_data{
	public:
		unsigned xi;
		unsigned yi;
		unsigned xf;
		unsigned yf;
		
		int n;
		int nSqrd;
		double sampling_size;
};

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
		// Read in the scene block
		else if( cmd == scene.begin_tag() ) { scene.read(ins); }
		// Read in the thread options
		else if( cmd == "") {

		}
	}
}

// thread pointer function
void *renderScene(void* threadArg){

	thread_data *data = (thread_data *) threadArg;
	
	/* Loop over the pixels */
	for(unsigned pixelY = data->yi; pixelY < data->yf; pixelY++){

		// if two backgrounds were specified, 
		//  calculate the new BG color at this row
		//if( scene.getHasTwoColors() )	
		//	scene.calcNextBGColor( pixelY, numYPixels );

		cerr << pixelY << "  ";

		for(unsigned pixelX = data->xi; pixelX < data->xf; pixelX++){

			//if(x == 47 && y == 73)
			//	testPixel = true;
			//else
			//	testPixel = false;

			gmVector3 pixelColor;
			bool permuteNumUsed[data->nSqrd];

			for( int b=0; b<(data->nSqrd); b++ )
				permuteNumUsed[b] = false;


			for(double u = 0; u < 1; u += data->sampling_size ){
				for(double v = 0; v < 1; v += data->sampling_size ){
					
					double x = (double)pixelX + u + (randDouble() * data->sampling_size),
						   y = (double)pixelY + v + (randDouble() * data->sampling_size);

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

					gmVector3 finalColor;

					// setup the ray
					thisRay.set_origin(view.getEye());
					thisRay.set_dir(ur*view.getVectorU() + vr*view.getVectorV() + wr*view.getVectorW());

					// the closest surface mat
					material* hitSurfaceMat = NULL;

					// randomly generate a time for this ray
					int permNum=-1;
					
					do{
						permNum = randInt(0, data->nSqrd);

						if( !permuteNumUsed[permNum] ){
							permuteNumUsed[permNum] = true;
							break;
						}
					}while(1);

					double time = (randDouble() + permNum) / (data->nSqrd);
					/* loop over the object list of the scene */
					hitSurfaceMat = scene.checkIntersections(thisRay, t0, t1, hit, time);

					// if we didn't hit anything
					if( hitSurfaceMat == NULL ){
						finalColor = scene.getBGColor();
					}
					// let the fun begin!
					else{
						gmVector3 tempCol,	// this is what returns from the reflection recursion
								  dirNorm = thisRay.get_dir_norm(),
								  r = dirNorm - 2 * ( dot(dirNorm, hit.getNormal()) ) * hit.getNormal();
						
						// the ray to be cast for reflection/refraction
						ray_t reflRay;
						reflRay.set_origin( thisRay.get_p() );
						reflRay.set_dir( r );

						//
						finalColor = scene.calcPointColor(hitSurfaceMat, thisRay, hit, 
								view.getEye(), 0, time);
						
						// clamp'em
						finalColor.clamp(0.0, 1.0);
					}

					// add the color to the running total
					pixelColor[0] += finalColor[0]/data->nSqrd;
					pixelColor[1] += finalColor[1]/data->nSqrd;
					pixelColor[2] += finalColor[2]/data->nSqrd;
				}
			}

			// set the color in the image array
			finalImage[ pixelX + (pixelY*view.getNumXPixels()) ] = pixelColor;
		}
	}

	return (void *)1;
}

/*****************************************************************************/
int main( int argc, char* argv[] )
{
	// Read in the data
    frame_read( cin );

	// .ppm header info
	cout << "P3" << endl;
	cout << view.getNumXPixels() << " " 
		<< view.getNumYPixels() << endl;
	cout << "255" << endl;

	unsigned numYPixels = view.getNumYPixels(),
			 numXPixels = view.getNumXPixels();

	// This, an array representing the final image.
	//	The threads will not output their colors, directly.  Instead,
	//	they will save their pixels' colors in the 2d array passed
	//	to them.  This is MutEx safe as they will never access the
	//	same memory.
	finalImage = new gmVector3[numXPixels*numYPixels];

	int n = view.getSamplingSize();
	int nSqrd = n*n;
	double sampling_size = 1.0/n;

	// the initial number of threads is 1
	int numThreads = 4;			//no multi-threading
	double numThreadsSqrt = 2.0;
	//double ARx, ARy;
	   
	// get the user's thread count. spec.
/*	if( argc > 2 ){

		// store the user's thread spec
/		numThreads = (int)argv[1];	//should be 1 or 4

		// if numThreads is 4
		if( numThreads == 4 ){
			numThreadsSqrt = 2.0;
		}
		// if numThreads is not 1 or 4
		else if( numThreads != 1){
			numThreads = 4;
			numThreadsSqrt = 2.0;
		}
		// else numThreads is 1 (no multi-threading)

		
		//
		// I betta validate that damn shit!
		//////

		/////
		// PHASE i
		////////////
		// Check that it's squareable
		///////////////

		// An integer square root will tell us that the thread 
		//  count spec is valid.
		numThreadsSqrt = sqrt(numThreads);

		// if the truncated sqrt of thread count over the untrunc-ed sqrt is not 1, 
		//  the square root is NOT VALID
		if( ((int)numThreadsSqrt)/numThreadsSqrt != 1.0 ){
			cerr << "Error:  " << numThreads << " is invalid: not squareable." << endl
				<< "Defaulting to 1 thread." << endl;

			numThreads = 1;
			numThreadsSqrt = 1.0;
			//break;
		}

		/////
		// PHASE ii
		////////////
		// Check the aspect ratio
		///////////////

		ARx = (numXPixels/numThreadsSqrt);
		ARy = (numYPixels/numThreadsSqrt);

		// now ensure that the aspect ratio can be split evenly in this way
		//  Here, if ARx and ARy are not ints, we have to default to 1 thread.
		if( ((int)ARx / ARx) != 1.0 || ((int)ARy / ARy) != 1.0 )
		{
			cerr << "Error:  The image size: " << numXPixels << "x" << numYPixels
				<< ", is not divisible into " << numThreads << " threads." << endl
				<< "Deafualting to 1 thread." << endl;

			numThreads = 1;
			numThreadsSqrt = 1.0;
			//break;
		}
		
	}*/

	cerr << "Number of threads to run: " << numThreads << " ." << endl;

	// calculate the increment
	int pixelX = 0,
		pixelY = 0;
	int xIncr = (int)(numXPixels/numThreadsSqrt);
	int yIncr = (int)(numYPixels/numThreadsSqrt);
	pthread_t renderCore[numThreads];
	thread_data* threadArgs = NULL;

	// spawn the threads.
	for( int tc=0; tc < numThreads; tc++){

		threadArgs = new thread_data();

		// initialize the thread data
		threadArgs->xi = pixelX;
		threadArgs->xf = pixelX + xIncr;

		threadArgs->yi = pixelY;
		threadArgs->yf = pixelY + yIncr;

		threadArgs->n = n;
		threadArgs->nSqrd = nSqrd;
		threadArgs->sampling_size = sampling_size;

		// fork a thread
		int iCreateReturn = pthread_create(&renderCore[tc], NULL, renderScene, (void*)threadArgs);

		//error check the return.
		if(iCreateReturn == EAGAIN){
			cout << "The system lacks the resources to create another thread" << endl;
			exit(-1);
		}

		// increment the pixelX and pixelY markers
		if( tc%2 == 0 ){
			pixelX += xIncr;
			//pixelY = 0;
		}
		else{
			pixelX = 0;
			pixelY += yIncr;
		}
	}

	// join the threads
	for( int p=0; p < numThreads; p++ )
		pthread_join( renderCore[p], NULL );

	// output the color
	for( unsigned finalY=0; finalY < numYPixels; finalY++ ){
		for( unsigned finalX=0; finalX < numXPixels; finalX++){

			// get "this pixel" color
			gmVector3 pixelColor = finalImage[finalX + (finalY*numXPixels)];
			// output the final color at this point
			cout << (int)( 255 * pixelColor[0] )
				<< " "
				<< (int)( 255 * pixelColor[1] ) 
				<< " "
				<< (int)( 255 * pixelColor[2] ) 
				<< " ";
		}
	}

	// destroy the stored image data
	delete [] finalImage;

	return( 0 );
}

void testThread( int id ){
	cerr << id << " ran." << endl;
}

