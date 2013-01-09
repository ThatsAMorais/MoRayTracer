// *************************************************************************
//    file: hatch_set.h
//  author: Alex Morais
// purpose: a class to hold one set of hatching laminae and the parameters
// created: 1-23-2007
// *************************************************************************

#ifndef HATCH_SET_H
#define HATCH_SET_H

#include <iostream>
#include <cstring>
#include <gmVec3.h>
#include <gmVec4.h>
#include <vector>

using namespace std;

class lamina{
	public:

		gmVector4* _e;	// plane coeffs
		double _a,		// plane offset
			   _c,		// weight factor
			   _d;		// weight factor

		char* begin_tag() { return "begin_lamina"; }
		char* end_tag() { return "end_lamina"; }

		bool read( std::istream& ins );
};

typedef vector <lamina*> LaminaVec;

class hatch_set{
	public:

		char* begin_tag() { return "begin_hatch"; }
		char* end_tag() { return "end_hatch"; }

		// .ray script parsing function
		bool read( std::istream& ins );

		double get_hf( void ){ return _hf; }
		double get_hb( void ){ return _hb; }
		int numOfLamina( void ){ return _laminaVec.size(); }
		lamina* getLamina( int index ){ return _laminaVec[index]; }

		// convenience accessors
		double get_a0( void ){ return _laminaVec[0]->_a; }
		double get_c0( void ){ return _laminaVec[0]->_c; }
		double get_d0( void ){ return _laminaVec[0]->_d; }

	private:
		LaminaVec _laminaVec;	// the first plane's coefficients
		double _hf,			// line thickness factor
			   _hb;			// line thickness term
			   // (h - hb)hf  =  line thickness modifier
};

#endif


