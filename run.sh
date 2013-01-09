
#!/bin/bash

# Runs the raytracer, converts the output image to png format, and displays it
# using Gnome image viewer

cat $1 | ./mray.exe > outfile.ppm
convert -flip outfile.ppm images/$2
eog images/$2
