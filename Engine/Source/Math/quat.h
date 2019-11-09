// **************************************************************************
// Crazy Dreamer, 2010
// **************************************************************************

#pragma once

#include "vec4.h"
#include "mat3.h"
#include "mat4.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

class SQMATH_API quat : public vec4
{
public:

quat();
quat(const float& _x, const float& _y, const float& _z, const float& _w);
quat(const vec3& _v, const float& _w);
quat(const vec4 &v);
quat(const mat3 &m);

quat conj() const;

void renormalize();

quat operator&(const quat& q);
void operator&=(const quat& q);

// —оздать из угла и оси
quat fromAxisAngle(vec3 axis, float angle);

// —оздать из угла
quat fromPhysicAngle(vec3 ph);

// —конвертировать в осевой угол
vec4 toAxisAngle(vec4& dst) const;

// —конвертировать в угол
vec3 toPhysicAngle(vec3& dst) const;

// ѕереназначить из углов Ёйлера, заданных координатами
quat fromEulerAngle ( float yaw, float pitch, float roll );

// ѕереназначить из углов Ёйлера, заданных координатами
quat fromEulerAngle ( vec3 e );

// —конвертировать в углы Ёйлера
vec3 toEulerAngle(vec3 &dst) const;

quat& fromRotationMatrixNV( const mat3 & m );

quat fromRotationMatrix ( const mat3& kRot );

mat3 toRotationMatrix() const;

quat fromRotationBetween( const vec3 & rotateFrom, const vec3 & rotateTo );

quat fromRotationLook( const vec3& from_look, const vec3& from_up,
    const vec3& to_look, const vec3& to_up);

//
// Quaternion multiplication with cartesian vector
// v' = q*v*q(star)
//
void multVec( const vec3&src, vec3&dst ) const;

void multVec( vec3& src_and_dst) const;

quat slerp(const quat& q, float t );

// ¬озвращает вектор направлени€
vec3 getDirVector() const;

vec3 apply( const vec3& v ) const;

};

} //namespace Math {

} //namespace Squirrel {
