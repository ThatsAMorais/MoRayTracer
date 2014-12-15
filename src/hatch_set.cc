// *************************************************************************
//    file: hatch_set.cc
//  author: Alex Morais
// purpose: a class to hold materials of surfaces(primitive, but under
// 			construction)
// created: 4-19-2007
// *************************************************************************

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <gmVec4.h>
#include <vector>
#include "hatch_set.h"

bool hatch_set::read( std::istream& ins ){
	
	std::string cmd;
	
	bool	seen_end_tag = false;
	bool	seen_lamina = false;
	bool	seen_hf = false;
	bool	seen_hb = false;

	// hatching lines
	_hf = 1.0;
	_hb = 0.0;

	// Loop and read until we reach the end of the view construct
	while( !ins.eof() && !seen_end_tag ){
		
		//
		ins >> cmd;
		//
		
		// Skip a comment line; ignore input until end-of-line		
		if( cmd == "#" ){
			ins.ignore( std::numeric_limits<int>::max(), '\n' ); 
		}
		// Detect end of the block
		else if( cmd == this->end_tag() ){
			seen_end_tag = true;
		}

		// Read in commands
		else if( cmd == "begin_lamina" ){

			// temporary holder of input
			lamina* newLamina = new lamina();

			if( ( newLamina->read(ins) ) && (numOfLamina() < 2) ){
				_laminaVec.push_back( newLamina );
				seen_lamina = true;
			}
		}
		else if( cmd == "hf" ){
			ins >> _hf;
			seen_hf = true;
		}
		else if( cmd == "hb" ){
			ins >> _hb;
			seen_hb = true;
		}
	}

	// Does this material have hatching lines?
	if( seen_lamina && seen_hf && seen_hb )
		return true;
	else
		return false;
}

bool lamina::read( std::istream& ins ){
	
	std::string cmd;
	
	bool	seen_end_tag = false;
	bool	seen_e = false;
	bool	seen_c = false;
	bool	seen_d = false;
	bool	seen_a = false;

	// hatching lines
	_a = 0.4;
	_e = NULL;
	_c = 1.0;
	_d = 1.0;

	// Loop and read until we reach the end of the view construct
	while( !ins.eof() && !seen_end_tag ){

		//
		ins >> cmd;
		//
		
		// Skip a comment line; ignore input until end-of-line		
		if( cmd == "#" ){
			ins.ignore( std::numeric_limits<int>::max(), '\n' ); 
		}
		// Detect end of the block
		else if( cmd == this->end_tag() ){
			seen_end_tag = true;
		}
		else if( cmd == "a" ){
			ins >> _a;
			seen_a = true;
		}
		else if( cmd == "e" ){
			gmVector4 e;
			ins >> e;
			_e = new gmVector4(e);
			seen_e = true;
		}
		else if( cmd == "c" ){
			ins >> _c;
			seen_c = true;
		}
		else if( cmd == "d" ){
			ins >> _d;
			seen_d = true;
		}
	}

	if( seen_e && seen_a && seen_c && seen_d )
		return true;
	else
		return false;
}

