//*****************************************************************************
//    file: surface.h
// created: 2-1-2005
//  author: J. Edward Swan II
// purpose: Function stubs for virtual class; these should not be called; they 
//          are just here to make the linker happy.
//*****************************************************************************

#include "surface.h"

using namespace std;

/*****************************************************************************/
surface_t::surface_t( void )
{
    //cout << "surface_t::constructor" << endl;
}
/*****************************************************************************/
surface_t::~surface_t( void )
{
    //cout << "surface_t::destructor" << endl;
}
/*****************************************************************************/
char* surface_t::begin_tag( void )
{
    //cout << "surface_t::begin_tag" << endl;
    return NULL;
}
/*****************************************************************************/
char* surface_t::end_tag( void )
{
    //cout << "surface_t::end_tag" << endl;
    return NULL;
}
/*****************************************************************************/
void surface_t::read( std::istream& ins )
{
    //cout << "surface_t::read" << endl;
}
/*****************************************************************************/
surface_t* surface_t::intersect( ray_t& ray, double t0, double t1, hit_t& hit, double time )
{
    //cout << "surface_t::intersect" << endl;
    return false;
}
/*****************************************************************************/
void surface_t::print( ostream& os )
{
    //os << "surface_t::print";
}
/*****************************************************************************/
gmVector3 surface_t::getNormal( gmVector3 rt )
{
	gmVector3 normal;

    return normal;
}
