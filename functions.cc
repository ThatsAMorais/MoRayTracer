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

//generates a psuedo-random double between 0.0 and 0.999...
double randDouble()
{
    return rand()/(double(RAND_MAX)+1);
}

//generates a psuedo-random double between min and max
double randDouble(double min, double max)
{
    if (min>max){
        return randDouble()*(min-max)+max;
    }
    else{
        return randDouble()*(max-min)+min;
    }
}

int randInt(int min, int max)
{
	if(min>max){
		return max + ( rand() % (min-max) );
	}
	else{
		return min + ( rand() % (max-min) );
	}
}



