#pragma once

#include <Math/vec3.h>
#include "macros.h"

namespace Squirrel {

using namespace Math;

class SQCOMMON_API LookAtObject//lamp, camera, etc.
{
protected:
	vec3 position;
	vec3 direction;

public:

	inline LookAtObject(void)
	{
		position = vec3(0.0f,0.0f,0.0f);
		direction = vec3(0.0f,0.0f,1.0f);
	}
	inline LookAtObject(vec3 p, vec3 d)
	{
		set( p, d );
	}
	inline ~LookAtObject(void)
	{

	}

	void turnLRDir(float alpha)
	{
		vec3 pol=vec3::Cartesian2Polar(direction);
		pol.y+=alpha;
		direction=vec3::Polar2Cartesian(pol);
	}

	void turnUDDir(float alpha)
	{
		vec3 pol=vec3::Cartesian2Polar(direction);
		if(fabsf(pol.x+alpha)<1.5f)
			pol.x+=alpha;
		direction=vec3::Polar2Cartesian(pol);
	}

	void movePosDirXZ(float step)
	{
		vec3 d(direction);
		d.normalize();
		d*=step;
		d.y=0;
		position+=d;
	}

	void moveRLPosDirXZ(float step)
	{
		vec3 d(direction);
		d.normalize();
		d*=step;
		d.y=0;
		float ttt=d.z;
		d.z=-d.x;
		d.x=ttt;
		position+=d;
	}

	void movePosDirXYZ(float step)
	{
		vec3 d(direction);
		d.normalize();
		d*=step;
		position+=d;
	}
public:

	inline void setPosition(vec3 p)		{	position = p;	}
	inline void setDirection(vec3 d)	{	direction = d;	}

	inline vec3 getPosition(void)	{	return position;	}
	inline vec3 getDirection(void)	{	return direction;	}

	inline void set( vec3 p, vec3 d )
	{
		position=p;
		direction=d;
	}
};

} //namespace Squirrel {