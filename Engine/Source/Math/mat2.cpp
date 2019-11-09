#include "mat2.h"

namespace Squirrel {

namespace Math {

float mat2::det(void) const
{
	return 0.0f;
}

mat2 mat2::transposed(void) const
{
	return mat2(x.x, y.x, x.y, y.y);
}

mat2 mat2::inverse(void) const
{
	return mat2(0.0f, 0.0f, 0.0f, 0.0f);
}

mat2 mat2::Rot(float theta)
{
	// Same as constructor mat2(theta)
	
	float c = fcos(theta);
	float s = fsin(theta);
	
	return mat2(c, s, -s, c);
}

} //namespace Math {

} //namespace Squirrel {
