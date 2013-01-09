// **************************************************************************
// file: 	functions.h
// author: 	Alex Morais
// purpose: To provide some general global functions to support ray tracer
// **************************************************************************

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cstdlib>
#include <gm.h>
#include "view.h"
#include "scene.h"

// new function: castRay
gmVector3 castRay(ray_t& thisRay, double t0, double t1,
		double time, int recDepth);

gmVector3 FourToThree(gmVector4* vec4);
gmVector4 ThreeToFour(gmVector3* vec3);
double randDouble();
double randDouble(double min, double max);
int randInt(int min, int max);

extern bool testPixel;
extern view_t view;
extern Scene scene;

#endif
