#include "vec4.h"

namespace Squirrel {

namespace Math {

vec4 vec4::normalized(void) const
{
	float l = len();
	if(l>0)	l=1/l;
	else	l=0;
	return (*this) * l;
}

void vec4::normalize(void)
{
	float l = len();
	if(l>0)	l=1/l;
	else	l=0;
	(*this) *= l;
}

void vec4::mix(const vec4& v1, const vec4& v2, const float f )
{
	(*this)=v1*(1-f)+v2*f;
}


} //namespace Math {

} //namespace Squirrel {
