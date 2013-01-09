/////////Final Assignment Readme:////////////

Execution Instructions:
	
	>> make
	>> cat <file.ray> | ./mray.exe > outfile.ppm
	>> convert -flip outfile.ppm output.png
	>> eog output.png			# here you could use the image viewer of your choice

...and that should do it.

Multi-threading:

	I have had much success with multi-threading in this assignment.  I obtained a 2x render speed increase.  Although, I have also experienced a rare bug where the ray tracer crashes when using more than 2 threads, yet re-rendering the image does not crash at the same place or at all.  I cannot determine, using debuggers, where the program is crashing, but there may be some thread-unsafe area in the code.  This is the only hitch.  Otherwise, the multi-threading produces a wonderful speed-up!

	The scheme I use divides the image into 20 blocks; 10 horizontal rows cut down the middle.  This limits the number of concurrent processes that can be run to render the image, but it is a small adjustment to the code to accomodate a smaller, more dynamic block size.

Computer Generated Copperplate Engravings:

	The copperplate images speak for themselves.  Its a simulation of ink on paper guided by engraved copper.  I have also implemented edge lines for both spheres and polygons.  Before my presentation I had not implemented silhouette edges on polygons, but I managed to get it in.  That completes all of the tasks for this segment of the project.

Images:
	- Image files are in /_images .
	- The corresponding ray files may not necessarily be in the state they were in when I made the images.  I used only these two to render copperplate engraving images
		-ass4_refract.ray
		-desert_1.ray

The 'n' component ( stochastic super-sampling ):
	In my .ray files I've added an 'n' value to the view parameters.  Maybe that isn't the most rational abstraction, but that's where I put it.  So, n is the number of rows and columns into which a pixel should be broken.  In other words, the samples per pixel, spp, equals n times n ( spp = n*n ).

Testing:
	Multi-threading:
		Use any ray file you want.  I've stored an assortment of old ray files in the zip with my code.
		To change the number of threads:
			(1) open "mray.cc"
			(2) Near the top of the file, locate the NUM_THREADS constant
			(3) You may set this value to any number between 1 and 20

		The multi-threading scheme does not support more than 20 threads.  Also, when watching the streaming output of numbers describing "pixels that have been rendered" and when only one render thread is enabled, don't be alarmed if the output stream pauses for a few moments.  The output image format is .ppm and the pixel values must be written in order.  

	Computer Generated Copperplate Engravings:

		You can use either of the ray files below:
			-ass4_refract.ray
			-desert_1.ray

		Or you can add a "begin_hatch ... begin_lamina .. end_lamina ... end_hatch" block to and object in another ray file.  Be warned about the parameters.  You may want to look at the two ray files above to get a grip on the parameters if you have trouble with your own inputs.  Take it from me, the parameters may be difficult to predict.

		To edit the properties of copperplates using the parameters in the scene file look for "begin_hatch" in the material description.  You can only specify 1 or 2 lamina planes for each object.  Any more and it would generate a mess.  The parameters are hf(float), hb(float), and for each lamina a(float), e(4d float), c(float), d(float).

		Parameter meanings:
			global hatch params:
				hf, hb - line thickness modifiers.  Computed by (h - hb) * hf.  h is relative to the ray traced color of the surface.
			individual lamina properties
				a - offset between planes.  This will control the frequency of hatching lines on the surface
				e - the coefficients of the plane equation.  a 4d vector.  I would suggest that you provide a large number for e4
					i.e. "e 1 3 3 30"
				c, d - scaling factors to weigh "this" lamina set.  It controls the intensity of "this" lamina set.  So, if you consider that the final pixel value, K, is a gray intensity which I round up if >= 0.5, this can be used to show preference for another lamina set over this one or vice versa.
			


