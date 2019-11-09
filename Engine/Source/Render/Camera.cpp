// Camera.cpp: implementation of the Camera class.
//
//////////////////////////////////////////////////////////////////////

#include "Camera.h"

namespace Squirrel {
namespace Render {

Camera * Camera::sMainCamera = 0;

Camera * Camera::GetMainCamera()
{
	return sMainCamera;
}

void Camera::setAsMain()
{
	sMainCamera = this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Camera::Camera()
{
	mUp			= vec3(0,1,0);
	mFov		= DEG2RAD * 90.0f;
	mAspect		= 1.0f;
	mNear		= 0.1f;
	mFar		= 500.0f;
	mSize		= 1.0f;
	mType		= Perspective;
	if(sMainCamera == 0)
	{
		setAsMain();
	}
}

Camera::Camera(EType type)
{
	mUp			= vec3(0,1,0);
	mFov		= DEG2RAD * 90.0f;
	mAspect		= 1.0f;
	mNear		= 0.1f;
	mFar		= 500.0f;
	mSize		= 1.0f;
	mType		= type;
}

Camera::~Camera()
{

}

void Camera::buildProjection(float left, float right, float bottom, float top, float neard, float fard)
{
	mNear	= neard;
	mFar	= fard;

	//calc mFov, mAspect and mSize for getters
	float ysize = top - bottom;
	float xsize = right - left;
	mFov = atanf ((ysize)*0.5f / mNear) * 2.0f;
	mAspect = xsize / ysize;
	mSize = ysize;

	//build camera
	build(left, right, bottom, top);
}

void Camera::buildProjection(float fov, float aspect, float neard, float fard)
{
	mNear	= neard;
	mFar	= fard;

	//store for getters
	mFov	= fov;
	mAspect	= aspect;

	//calc bounds
    float top		= mNear * tanf( mFov * 0.5f );
    float right		= top * mAspect;
    float bottom	= -top;
    float left		= -right;
    
	//build camera
	build(left, right, bottom, top);
}

void Camera::build(float left, float right, float bottom, float top)
{
	//build projection matrix
	if(mType == Orthographic)
	{
		mProjectionMatrix = calcOrthoMatrix(left, right, bottom, top);
	}
	else if(mType == Perspective)
	{
		mProjectionMatrix = calcFrustumMatrix(left, right, bottom, top);
	}
	update();
}

int	Camera::computeNearPointMask ( const vec3& n )
{
    if ( n.x > 0.0f )
        if ( n.y > 0.0f )
            if ( n.z > 0.0f )
                return 0;
            else
                return 4;
        else
            if ( n.z > 0.0f )
                return 2;
            else
                return 6;
    else
        if ( n.y > 0.0f )
            if ( n.z > 0.0f )
                return 1;
            else
                return 5;
        else
            if ( n.z > 0.0f )
                return 3;
            else
                return 7;
}

mat4 Camera::CalcOrthoMatrix(float left, float right, float bottom, float top, float neard, float fard)
{
	float r_m_l = right - left;
	float t_m_b = top - bottom;
	float f_m_n = fard - neard;

	float r_p_l = right + left;
	float t_p_b = top + bottom;
	float f_p_n = fard + neard;

	float tx = (r_p_l * -1)/r_m_l;
	float ty = (t_p_b * -1)/t_m_b;
	float tz = (f_p_n * -1)/f_m_n;

	return mat4 
	(
		2.0f/r_m_l,	0.0f,		0.0f,			tx,
		0.0f,		2.0f/t_m_b,	0.0f,			ty,
		0.0f,		0.0f,		-2.0f/f_m_n,	tz,
		0.0f,		0.0f,		0.0f,			1.0f//here should be -1
	);
}

mat4 Camera::CalcFrustumMatrix(float left, float right, float bottom, float top, float neard, float fard)
{
	float n2 = 2.0f * neard;
	float f2 = 2.0f * fard;

	float r_m_l = right - left;
	float t_m_b = top - bottom;
	float f_m_n = fard - neard;

	float r_p_l = right + left;
	float t_p_b = top + bottom;
	float f_p_n = fard + neard;

	float tmp = -1.0f/f_m_n;

	return mat4 
	(
		n2/r_m_l,	0.0f,		r_p_l/r_m_l,	0.0f,
		0.0f,		n2/t_m_b,	t_p_b/t_m_b,	0.0f,
		0.0f,		0.0f,		f_p_n*tmp,		f2*neard*tmp,
		0.0f,		0.0f,		-1.0f,			0.0f
	);
}

mat4 Camera::CalcLookAtMatrix(const vec3& pos, const vec3& dir, const vec3& up)
{
	vec3 direction = dir.normalized();
	vec3 right = (direction ^ up).normalized();//calc right direction
	vec3 newUp = (right ^ direction).normalized();//recompute up
	vec3 invDir = direction * -1;//calc inverted forward direction

	mat4 orient 
	(
		right.x,	right.y,	right.z,	0.0f,
		newUp.x,	newUp.y,	newUp.z,	0.0f,
		invDir.x,	invDir.y,	invDir.z,	0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
	);

	mat4 offset;
	offset.identity();
	offset.setTranslate( pos * -1.0f );

	return (orient * offset);
}

mat4 Camera::calcOrthoMatrix(float left, float right, float bottom, float top)
{
	vec3 rightVec = (direction ^ mUp).normalized();
	vec3 newUp = (rightVec ^ direction).normalized();

	vec3 fc = position + direction * mFar;
	vec3 nc = position + direction * mNear;

	mPoints[0] = nc + newUp*bottom	+ rightVec*left;
	mPoints[1] = nc + newUp*top		+ rightVec*left;
	mPoints[2] = nc + newUp*top		+ rightVec*right;
	mPoints[3] = nc + newUp*bottom	+ rightVec*right;

	mPoints[4] = fc + newUp*bottom	+ rightVec*left;
	mPoints[5] = fc + newUp*top		+ rightVec*left;
	mPoints[6] = fc + newUp*top		+ rightVec*right;
	mPoints[7] = fc + newUp*bottom	+ rightVec*right;

	return CalcOrthoMatrix(left, right, bottom, top, mNear, mFar);
}

mat4 Camera::calcFrustumMatrix(float left, float right, float bottom, float top)
{
	return CalcFrustumMatrix(left, right, bottom, top, mNear, mFar);
}

mat4 Camera::calcLookAtMatrix()
{
	return CalcLookAtMatrix(position, direction, mUp);
}

void Camera::buildFinal()
{
	//build loakAt matrix
	mat4 lookAt = calcLookAtMatrix();

	//final projection matrix
	mFinalMatrix = mProjectionMatrix * lookAt;
}

void Camera::update()
{
	buildFinal();
	buildPlanes();
	buildPoints();
}

void Camera::buildPoints()//for visibility detection and visualization
{
	//direction should be normalized

	if(mType == Orthographic)
	{
	}
	else
	{
		vec3 right = (direction ^ mUp).normalized();
		vec3 newUp = (right ^ direction).normalized();

		vec3 fc = position + direction * mFar;
		vec3 nc = position + direction * mNear;

		float tanHFov = tanf(mFov*0.5f);

		// these heights and widths are half the heights and widths of
		// the near and far plane rectangles
		float near_height = tanHFov * mNear;
		float near_width = near_height * mAspect;
		float far_height = tanHFov * mFar;
		float far_width = far_height * mAspect;

		mPoints[0] = nc - newUp*near_height - right*near_width;
		mPoints[1] = nc + newUp*near_height - right*near_width;
		mPoints[2] = nc + newUp*near_height + right*near_width;
		mPoints[3] = nc - newUp*near_height + right*near_width;

		mPoints[4] = fc - newUp*far_height - right*far_width;
		mPoints[5] = fc + newUp*far_height - right*far_width;
		mPoints[6] = fc + newUp*far_height + right*far_width;
		mPoints[7] = fc - newUp*far_height + right*far_width;
	}
}


void Camera::buildPlanes()
{
	mat4 clip = mFinalMatrix.transposed();
	
	/* right plane */
	mPlanes[0][0] = clip.x.w - clip.x.x;
	mPlanes[0][1] = clip.y.w - clip.y.x;
	mPlanes[0][2] = clip.z.w - clip.z.x;
	mPlanes[0][3] = clip.w.w - clip.w.x;
	
	mPlanes[0].normalize();
	mPlanesMask[0] = computeNearPointMask( mPlanes[0].getVec3() );
	
	/* left plane */
	mPlanes[1][0] = clip.x.w + clip.x.x;
	mPlanes[1][1] = clip.y.w + clip.y.x;
	mPlanes[1][2] = clip.z.w + clip.z.x;
	mPlanes[1][3] = clip.w.w + clip.w.x;
	
	mPlanes[1].normalize();
	mPlanesMask[1] = computeNearPointMask( mPlanes[1].getVec3() );
	
	/* down plane */
	mPlanes[2][0] = clip.x.w + clip.x.y;
	mPlanes[2][1] = clip.y.w + clip.y.y;
	mPlanes[2][2] = clip.z.w + clip.z.y;
	mPlanes[2][3] = clip.w.w + clip.w.y;
	
	mPlanes[2].normalize();
	mPlanesMask[2] = computeNearPointMask( mPlanes[2].getVec3() );
	
	/* up plane */
	mPlanes[3][0] = clip.x.w - clip.x.y;
	mPlanes[3][1] = clip.y.w - clip.y.y;
	mPlanes[3][2] = clip.z.w - clip.z.y;
	mPlanes[3][3] = clip.w.w - clip.w.y;
	
	mPlanes[3].normalize();
	mPlanesMask[3] = computeNearPointMask( mPlanes[3].getVec3() );
	
	/* back plane */
	mPlanes[4][0] = clip.x.w - clip.x.z;
	mPlanes[4][1] = clip.y.w - clip.y.z;
	mPlanes[4][2] = clip.z.w - clip.z.z;
	mPlanes[4][3] = clip.w.w - clip.w.z;
	
	mPlanes[4].normalize();
	mPlanesMask[4] = computeNearPointMask( mPlanes[4].getVec3() );
	
	/* front plane */
	mPlanes[5][0] = clip.x.w + clip.x.z;
	mPlanes[5][1] = clip.y.w + clip.y.z;
	mPlanes[5][2] = clip.z.w + clip.z.z;
	mPlanes[5][3] = clip.w.w + clip.w.z;
	
	mPlanes[5].normalize();
	mPlanesMask[5] = computeNearPointMask( mPlanes[5].getVec3() );
}

bool Camera::isPointIn(vec3 pt) const
{
	for(int p = 0; p < FRUSTUM_PLANES; p++ )
	if( mPlanes[p][0] * pt.x + mPlanes[p][1] * pt.y + mPlanes[p][2] * pt.z + mPlanes[p][3] <= 0 )
		return false;
	return true;
}

bool Camera :: isAABBIn ( const AABB& box ) const
{
	for ( register int i = 0; i < FRUSTUM_PLANES; i++ )
		if ( box.classify ( mPlanes [i], mPlanesMask[i] ) == Plane::IN_BACK )
			return false;
	return true;
}

bool Camera::isSphereIn(vec3 pos, float radius, bool sign) const
{
   for(int p = 0; p < FRUSTUM_PLANES; p++ )
      if( mPlanes[p][0] * pos.x + mPlanes[p][1] * pos.y + mPlanes[p][2] * pos.z + mPlanes[p][3] <= -radius )
         return false;
   return true;
}

float Camera::isSphereIn(vec3 pos, float radius, float sign) const
{
   float d;
   for(int p = 0; p < FRUSTUM_PLANES; p++ )
   {
      d = mPlanes[p][0] * pos.x + mPlanes[p][1] * pos.y + mPlanes[p][2] * pos.z + mPlanes[p][3];
      if( d <= -radius )
         return 0;
   }
   if(sign==0)	d+=radius;
   return d;
}

int Camera::isSphereIn(vec3 pos, float radius, int sign) const
{
	int c = 0;
	float d;
	for(int p = 0; p < FRUSTUM_PLANES; p++ )
	{
	   d = mPlanes[p][0] * pos.x + mPlanes[p][1] * pos.y + mPlanes[p][2] * pos.z + mPlanes[p][3];
	   if( d <= -radius )
		  return 0;
	   if( d > radius )
		  c++;
	}
	return (c == 6) ? 2 : 1;
}

bool Camera::isCubeIn(vec3 cubeVerts[CUBE_VERTS_NUM]) const
{
	//TODO: not sure if it works
	int i, j;
	for(i = 0; i < FRUSTUM_PLANES; ++i)
	{
		for(j = 0; j < CUBE_VERTS_NUM; ++j)
		{
			if( (mPlanes[i].normal() * cubeVerts[j]) + mPlanes[i].distance() >= 0 )
			{
				break;
			}
		}
		if(j == 8)
		{
			return false;
		}
	}
	return true;
}

vec3 Camera::unProject(vec3 screenPoint, tuple4i viewport) const
{
	mat4 inv = mFinalMatrix.inverse();

	vec4 in;

	//Transformation of normalized coordinates between -1 and 1
	in.x	= (screenPoint.x - (float)viewport[0]) / (float)viewport[2] * 2.0f - 1.0f;
	in.y	= (screenPoint.y - (float)viewport[1]) / (float)viewport[3] * 2.0f - 1.0f;
	in.z	= 2.0f * screenPoint.z - 1.0f;
	in.w	= 1.0f;

	//Objects coordinates
	vec4 out = inv * in;

	if(out.w != 0.0f) out.w = 1.0f / out.w;

	vec3 result;

	result.x = out.x * out.w;
	result.y = out.y * out.w;
	result.z = out.z * out.w;

	return result;
}

vec3 Camera::project(vec3 worldPoint, tuple4i viewport) const
{
	vec4 vec = mFinalMatrix * vec4(worldPoint, 1.0f);

	if(vec.w == 0) 
		return vec3::Zero(); 

	vec.x /= vec.w;
	vec.y /= vec.w;
	vec.z /= vec.w;

    /* Map x, y and z to range 0-1 */
    vec.x = vec.x * 0.5f + 0.5f;
    vec.y = vec.y * 0.5f + 0.5f;
    vec.z = vec.z * 0.5f + 0.5f;

    /* Map x,y to viewport */
    vec.x = vec.x * viewport[2] + viewport[0];
    vec.y = vec.y * viewport[3] + viewport[1];

	return vec.getVec3();
}

Ray Camera::getRayFromScreenPoint(vec2 screenPoint, tuple4i viewport) const
{
	Ray result;

	vec3 screenPoint3d(screenPoint.x, screenPoint.y, 0.0f);

	//calc ray start
	screenPoint3d.z = mNear;
	vec3 start = unProject(screenPoint3d, viewport);
	
	//calc ray end
	screenPoint3d.z = mFar;
	vec3 end = unProject(screenPoint3d, viewport);

	//calc ray direction
	result.mDirection = (end - start).normalized();

	result.mOrigin = start;

	return result;
}

}//namespace Render { 
}//namespace Squirrel {

