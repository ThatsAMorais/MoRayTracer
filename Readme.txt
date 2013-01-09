/////////Assignment 3 Readme:////////////

Execution Instructions:
	
	>> make
	>> cat <file.ray> | ./mray.exe > outfile.ppm
	>> convert -flip outfile.ppm output.png
	>> eog output.png			# here you could use the image viewer of your choice

...and that should do it.

Images:
	- Image files are in /_images .
	- The corresponding ray files may not necessarily be in the state they were in when I made the images.  I experimented with several different gloss values and transformations.  Some of the output IS the same as the ray file, such as the file dave posted which is in this .zip file under ass3_temporal.ray(sorry about the name. heh).

the 'n' component:
	In my .ray files I've added an 'n' value to the view parameters.  Maybe that isn't the most rational abstraction, but that's where I put it.  So, n is the number of rows and columns into which a pixel should be broken.  In other words, the samples per pixel, spp, equals n times n ( spp = n*n ).  I realize that this, also, may be a mistake to implement this way.  As a result, I felt it deserved explanation.

Testing:
	If you want to render thumbnails of desert.ray or pyramid.ray be sure to alter the 'n' and 'xres' values.  I rendered three enormous 1000x1000 images with 256 spp, 16spp, and 1spp(no anti-aliasing).  Just remember to consider that spp = n*n with respect to the 'n' component in the view parameters of the .ray file.

/* desert.ray */
This is a ray file of my own design.
Here's a component-based description of the scene in desert.ray
-----------------------------------------------------------------------------------------------------------------------------------
::		Surfaces		::	reflectivity		::	gloss	::	time_xforms 	::
-----------------------------------------------------------------------------------------------------------------------------------
close small blue sphere		yes : <1,1,1>		no				no
left red sphere(near pyramid)	yes : <1,1,1>		no				no
right red sphere				yes : <.8,.8,.8>	     yes : 0.3			no
far, high blue sphere			yes : <1,1,1>	     	no				no
the sun						no		     yes : 0.8			yes
pyramid					yes : <1,1,1>	     yes : 0.05			no
the sand					yes : <.1,.1,.1>		no				yes

//////////////////////////////////
