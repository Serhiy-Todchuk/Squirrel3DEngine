#pragma once

#include <common/common.h>
#include <common/LookAtObject.h>
#include <Math/Ray.h>
#include <Math/AABB.h>
#include "macros.h"

namespace Squirrel {
namespace Render { 

using namespace Math;

class SQRENDER_API Camera: 
	public LookAtObject
{
	static Camera * sMainCamera;

public://consts
	static const int FRUSTUM_PLANES = 6;
	static const int FRUSTUM_POINTS = 8;
	static const int CUBE_VERTS_NUM = 8;

	enum EType
	{
		Perspective,
		Orthographic
	};

public://ctor/dtor
	Camera();
	Camera(EType type);
	virtual ~Camera();

public://methods

	//main methods
	static Camera * GetMainCamera();
	void setAsMain();

	//build methods
	void buildProjection(float fov, float aspect, float neard, float fard);
	void buildProjection(float left, float right, float bottom, float top, float neard, float fard);
	void update();

	//frustrum interceprtion methods
	int		isSphereIn(vec3 pos, float radius, int sign) const;
	float	isSphereIn(vec3 pos, float radius, float sign) const;
	bool	isSphereIn(vec3 pos, float radius, bool sign) const;
	bool	isAABBIn(const AABB& box) const;
	bool	isPointIn(vec3 p) const;
	bool	isCubeIn(vec3 cubeVerts[CUBE_VERTS_NUM]) const;

	Ray getRayFromScreenPoint(vec2 screenPoint, tuple4i viewport) const;
	vec3 unProject(vec3 screenPoint, tuple4i viewport) const;
	vec3 project(vec3 worldPoint, tuple4i viewport) const;

	//getters
	inline float		getFov(void)		const	{ return mFov;	}
	inline float		getAspect(void)		const	{ return mAspect;	}
	inline float		getViewHeight(void)	const	{ return mSize;	}
	inline float		getNear(void)		const	{ return mNear;	}
	inline float		getFar(void)		const	{ return mFar;	}
	inline const mat4&	getProjMatrix(void)	const	{ return mProjectionMatrix; }
	inline const mat4&	getFinalMatrix(void)const	{ return mFinalMatrix; }
	inline const vec3&	getPoint(int fp)	const	{ ASSERT((fp>=0 && fp<FRUSTUM_POINTS)); return mPoints[fp]; }
	inline const Plane&	getPlane(int fp)	const 	{ ASSERT((fp>=0 && fp<FRUSTUM_PLANES)); return mPlanes[fp]; }

	//setters
	inline void		setUp(vec3 up)		{ mUp=up; }
	inline void		setSize(float s)	{ mSize=s; }
	inline void		setType(EType t)	{ mType=t; }

	//utilites
	static mat4 CalcLookAtMatrix(const vec3& pos, const vec3& dir, const vec3& up);
	static mat4 CalcOrthoMatrix(float left, float right, float bottom, float top, float neard, float fard);
	static mat4 CalcFrustumMatrix(float left, float right, float bottom, float top, float neard, float fard);

private://methods

	//build methods
	void	buildFinal();
	void	buildPlanes();
	void	buildPoints();
	void	build(float left, float right, float bottom, float top);

	//utils
	int		computeNearPointMask ( const vec3& n );
	mat4	calcFrustumMatrix(float left, float right, float bottom, float top);
	mat4	calcOrthoMatrix(float left, float right, float bottom, float top);
	mat4	calcLookAtMatrix();

private://members

	EType	mType;

	vec3	mUp;

	float	mFov;//field of view in radians
	float	mAspect;
	float	mNear;
	float	mFar;
	float	mSize;

	mat4	mProjectionMatrix;
	mat4	mFinalMatrix;

	vec3	mPoints[FRUSTUM_POINTS];
	Plane	mPlanes[FRUSTUM_PLANES];
	int		mPlanesMask[FRUSTUM_PLANES];
};

}//namespace Render { 
}//namespace Squirrel {
