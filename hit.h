//*****************************************************************************
//    file: hit.h
// created: 1-30-2005
//  author: J. Edward Swan II
// purpose: Hold needed information when a ray hits a surface
//*****************************************************************************

#ifndef HIT_H
#define HIT_H

#include <iostream>

class hit_t 
{
	public:
		double getT( void ){ return t; }
		void setT( double t_val ){ t = t_val; }
	private:
		double t;     // Parametric ray distance where hit occurs
};

#endif
