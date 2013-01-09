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
#include <errno.h>

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

// the number of threads to create
#define NUM_THREADS 8

view_t view;       // Hold the view
Scene scene;
static gmVector3*	finalImage;
static bool*		renderedPixels;
static int 			numBlocksLeft = 0;
static int 			pixelX = 0;
static int 			pixelY = 0;
static int 			yIncr = 0;
static int 			xIncr = 0;
static int 			n = 4;
static int 			nSqrd = 16;
static double 		sampling_size = 4.0;

// the greatest rendered pixel


//MUTEX attribute
pthread_mutexattr_t g_tMutex_attr;

//MUTEX
pthread_mutex_t g_tMutex;

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

thread_data* getNextBlock( void ){
	
	thread_data* nextBlock = new thread_data();

	// initialize the thread data
	nextBlock->xi = pixelX;
	nextBlock->xf = pixelX + xIncr;

	nextBlock->yi = pixelY;
	nextBlock->yf = pixelY + yIncr;

	nextBlock->n = n;
	nextBlock->nSqrd = nSqrd;
	nextBlock->sampling_size = sampling_size;


	// increment the pixelX and pixelY markers
	if( pixelX == 0 ){
		pixelX += xIncr;
		//pixelY = 0;
	}
	else{
		pixelX = 0;
		pixelY += yIncr;
	}

	numBlocksLeft += -1;
	return nextBlock;
}

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

// This ptr function polls a bool array and
//  writes the pixels to the output file
//  as they are rendered 
/*void *writeImage(void* threadArg){
	
}*/

// This ptr function writes pixels to the output .ppm as
//  they are rendered and inserted into the finalImage 
//  AND in the right order
void* writeImage(void* threadArg){

	cerr << endl << "ImageWriter started" << endl;
	int numPixels = (int)threadArg;

	//int progIncr = (int)( numPixels/10 );
	cerr << "Pixels to render := " << numPixels << endl;

	int colorToWrite = 0;

	//cerr << " ================== ====== Progress ====== ================== " << endl;

	while( colorToWrite < numPixels ){

		// if the "next" pixel is ready to be written...
		if( renderedPixels[colorToWrite] == true ){

			// lock access
			pthread_mutex_lock(&g_tMutex);

			//cerr << "Writing pixel " << colorToWrite << endl;

			// retrieve the color at the local index
			gmVector3 pixelColor = finalImage[ colorToWrite ];

			// output the final color at this point
			cout << (int)( 255 * pixelColor[0] )
				<< " "
				<< (int)( 255 * pixelColor[1] ) 
				<< " "
				<< (int)( 255 * pixelColor[2] ) 
				<< " ";

			// increment the local index
			colorToWrite++;

			// unlock access
			pthread_mutex_unlock(&g_tMutex);
		}

		
	}

	cerr << endl << endl << "** ImageWriter finished **" << endl;

	return (void *)1;
}

// This ptr function assigns itself an image block,
//  renders it, then gets another until there are 
//  none.
void *renderScene(void* threadArg){

	cerr << endl << " Render thread started ";

	thread_data *data = NULL; //(thread_data *) threadArg;

	while(1){
		
		// get the next block
		/////

		if(numBlocksLeft != 0){
			
			// access the global members mutually exclusively
			pthread_mutex_lock(&g_tMutex);

			cerr << " Getting a new block ";

			// delete the old thread data
			delete data;

			// get the next block
			data = getNextBlock();

	
			// end the locked code.
			pthread_mutex_unlock(&g_tMutex);
		}
		else
			break;

		/* Loop over the pixels */
		for(unsigned pixelY = data->yi; pixelY < data->yf; pixelY++){

			// if two backgrounds were specified, 
			//  calculate the new BG color at this row
			//if( scene.getHasTwoColors() )	
			//	scene.calcNextBGColor( pixelY, numYPixels );

			for(unsigned pixelX = data->xi; pixelX < data->xf; pixelX++){

				/*if(pixelX == 86 && pixelY == 143)
					testPixel = true;
				else
					testPixel = false;
				*/

				gmVector3 pixelColor;
				bool permuteNumUsed[data->nSqrd];

				for( int b=0; b<(data->nSqrd); b++ )
					permuteNumUsed[b] = false;


				for(double u = 0.0; u < 1.0; u += data->sampling_size ){
					for(double v = 0.0; v < 1.0; v += data->sampling_size ){
						
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

						// setup the ray
						thisRay.set_origin(view.getEye());
						thisRay.set_dir(ur*view.getVectorU() + vr*view.getVectorV() + wr*view.getVectorW());
						// set the refractive index to that of air
						//thisRay.set_refr_index(1.0);

						// randomly generate a time for this ray
						int permNum = -1;
						
						do{
							permNum = randInt(0, data->nSqrd);

							if( !permuteNumUsed[permNum] ){
								permuteNumUsed[permNum] = true;
								break;
							}
						}while(1);

						// calculate the time at which to render a surface hit
						//  by this ray(if it hits a surface)
						double time = (randDouble() + permNum) / (data->nSqrd);

						// cast the ray to get the color at this 
						//  (super-sampled) location
						gmVector3 finalColor = castRay(thisRay, 
								0, std::numeric_limits<double>::infinity(),
								time, 0);	

						// add the color to the running total
						pixelColor[0] += finalColor[0]/data->nSqrd;
						pixelColor[1] += finalColor[1]/data->nSqrd;
						pixelColor[2] += finalColor[2]/data->nSqrd;
					}
				}

				// set the color in the image array
				int currentPixel = pixelX + (pixelY*view.getNumXPixels());

				// lock access
				//pthread_mutex_lock(&g_tMutex);

				finalImage[ currentPixel ] = pixelColor;

				renderedPixels[currentPixel] = true;

				// unlock access
				//pthread_mutex_unlock(&g_tMutex);
			}

			cerr << pixelY << " || ";
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
	int totalPixels = numXPixels*numYPixels;
	finalImage = new gmVector3[totalPixels];
	renderedPixels = new bool[totalPixels];

	for(int b=0; b < totalPixels; b++)
		renderedPixels[b] = false;

	// set some globals
	n = view.getSamplingSize();
	nSqrd = n*n;
	sampling_size = 1.0/n;
	
	cerr << "Number of threads to run: " << NUM_THREADS << " ." << endl;

	//set more globals
	yIncr = (int)(numYPixels/10);
	xIncr = (int)(numXPixels/2);
	numBlocksLeft = 20;
	pthread_t renderCore[NUM_THREADS];

	//////
	//
	// a word from our sponsor...
	cerr << endl 
		<< "The numbers you're seeing are the rows of the image " << endl 
		<< "that have been rendered so far.  The threads all have " << endl
		<< "the task of rendering blocks of the image.  You are seeing " << endl
		<< "two of each number because the scanline blocks are split in " << endl
		<< "half, vertically.  Thus each row of the image has two parts, " 
		<< endl	<< "rendered separately."
		<< endl;



	// spawn the rendering threads. //
	for( int tc=0; tc < NUM_THREADS; tc++){

		// fork a thread
		int iCreateReturn = pthread_create(&renderCore[tc], NULL, renderScene, NULL);

		//error check the return.
		if(iCreateReturn == EAGAIN){
			cout << "The system lacks the resources to create another thread" << endl;
			exit(-1);
		}
	}
	//////

	// spawn the image writing threads //
	pthread_t imageWriter;
	int iCreateWriterReturn = pthread_create(&imageWriter, NULL, writeImage, 
			(void*)(totalPixels));

	if(iCreateWriterReturn == EAGAIN){
		cout << "The system lacks the resources to create the imageWriter thread" << endl;
		exit(-1);
	}

	// join the threads
	for( int p=0; p <= NUM_THREADS; p++ ){
		if( p == NUM_THREADS )
			pthread_join( imageWriter, NULL );
		else
			pthread_join( renderCore[p], NULL );
	}

	// image writer finishes writing the image
	//  ....
	//  ....

	// destroy the stored image data
	delete [] finalImage;

	return( 0 );
}


