// filename: functions.cc
// created: Feb 22, 2007
// description:
// 	the definitions of the utilitary functions

#include "functions.h"

gmVector3 FourToThree(gmVector4* vec4){
	gmVector3 vec3;

	vec3[0] = (*vec4)[0];
	vec3[1] = (*vec4)[1];
	vec3[2] = (*vec4)[2];

	return vec3;
}

gmVector4 ThreeToFour(gmVector3* vec3){
	gmVector4 vec4;

	vec4[0] = (*vec3)[0];
	vec4[1] = (*vec3)[1];
	vec4[2] = (*vec3)[2];
	vec4[3] = 1.0;

	return vec4;
}

