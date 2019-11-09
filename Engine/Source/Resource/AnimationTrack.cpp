#include "AnimationTrack.h"
#include <Math/mathTypes.h>
#include <Common/types.h>
#include <Common/macros.h>

namespace Squirrel {

namespace Resource { 

using namespace Math;

// the following function i got from the collada specification, but it's intended for the "BEZIER" type of interpolation, which works fine by the way..

/**
* Returns the approximated parameter of a parametric curve for the value X
* @param atX At which value should the parameter be evaluated
* @param P0_X The first interpolation point of a curve segment
* @param C0_X The first control point of a curve segment
* @param C1_X The second control point of a curve segment
* @param P1_x The second interpolation point of a curve segment
* @return The parametric argument that is used to retrieve atX using the parametric function representation of this curve
*/

float ApproximateCubicBezierParameter (
         float atX, float P0_X, float C0_X, float C1_X, float P1_X ) {

	const double aproximationEpsilon = 1.0e-09;
	const double verySmall = 1.0e-20;
	const int maximumIterations = 1000;

	if (atX - P0_X < verySmall)
		return 0.0;

	if (P1_X - atX < verySmall)
		return 1.0;

	long iterationStep = 0;

	float u = 0.0f; float v = 1.0f;

	//iteratively apply subdivision to approach value atX
	while (iterationStep < maximumIterations)
	{
		// de Casteljau Subdivision.
		double a = (P0_X + C0_X)*0.5f;
		double b = (C0_X + C1_X)*0.5f;
		double c = (C1_X + P1_X)*0.5f;
		double d = (a + b)*0.5f;
		double e = (b + c)*0.5f;
		double f = (d + e)*0.5f; //this one is on the curve!

		//The curve point is close enough to our wanted atX
		if (fabs(f - atX) < aproximationEpsilon)
		   return clamp((u + v)*0.5f, 0.0f, 1.0f);

		//dichotomy
		if (f < atX) 
		{
		   P0_X = (float)f;
		   C0_X = (float)e;
		   C1_X = (float)c;
		   u = (u + v)*0.5f;
		} 
		else 
		{
		   C0_X = (float)a; C1_X = (float)d; P1_X = (float)f; v = (u + v)*0.5f;
		}

		iterationStep++;
	}
	return clamp((u + v)*0.5f, 0.0f, 1.0f);
}

template <class T>
T SplineEqCubicBezier(T P0, T C0, T C1, T P1, float s)
{
	float s1 = (1-s);
	float s2 = s1*s1;
	float ss = s*s;

	return P0*(s2*s1) + C0*(3*s*s2) + C1*(3*ss*s1) + P1*(s*ss);
}

template <class T>
T SplineEqCubicHermite(T P0, T T0, T T1, T P1, float s)
{
	float s2 = s*s;
	float s3 = s*s2;

	return P0*(2*s3-3*s2+1) + T0*(s3-2*s2+s) + P1*(-2*s3+3*s2) + T1*(s3-s2);
}

float InterpolateCubicBezier(float P0, float C0, float C1, float P1, float s)
{
	mat4 bezierMatrix(	-1,  3, -3,  1,
						 3, -6,  3,  0,
						-3,  3,  0,  0,
						 1,  0,  0,  0	);
	float ss = s*s;
	vec4 S( s*ss, ss, s, 1 );
	vec4 C( P0, C0, C1, P1 );

	return (bezierMatrix * C) * S;
}

float InterpolateCubicHermite(float P0, float P1, float T0, float T1, float s)
{
	mat4 hermiteMatrix(	 2, -2,  1,  1,
						-3,  3, -2, -1,
						 0,  0,  1,  0,
						 1,  0,  0,  0	);
	float ss = s*s;
	vec4 S( s*ss, ss, s, 1 );
	vec4 C( P0, P1, T0, T1 );

	return (hermiteMatrix * C) * S;
}

float InterpolateBSpline(float Pm1, float P0, float P1, float P2, float s)
{
	const float u = 1.0f/6;
	mat4 bezierMatrix(	-1,  3, -3,  1,
						 3, -6,  3,  0,
						-3,  3,  0,  0,
						 1,  0,  0,  0	);
	mat4 splineMatrix = bezierMatrix * u;

	float ss = s*s;
	vec4 S( s*ss, ss, s, 1 );
	vec4 C( Pm1, P0, P1, P2 );

	return (splineMatrix * C) * S;
}

float InterpolateCardinal(float Pm1, float P0, float P1, float P2, float s, float tension)
{
	float t = (1.0f-tension)/2;
	mat4 splineMatrix(	-t, 2-t, t-2,   1,
					   2*t, t-3,3-2*t,1-t,
						-t,   0,   t,   0,
						 0,   1,   0,   0	);
	float ss = s*s;
	vec4 S( s*ss, ss, s, 1 );
	vec4 C( Pm1, P0, P1, P2 );

	return (splineMatrix * C) * S;
}


AnimationTrack::AnimationTrack(float * target, int targetComponentsNum, int framesNum): 
	mTarget(target),
	mTargetIndices(targetComponentsNum, -1),
	mTimeline(framesNum, 0), mFramesData(framesNum, NULL),
	mInterpolationType(LINEAR)
{
	for(int i = 0; i < framesNum; ++i)
	{
		mFramesData[i] = new float[targetComponentsNum];
	}
}

AnimationTrack::~AnimationTrack(void)
{
	for(uint i = 0; i < mFramesData.size(); ++i)
	{
		DELETE_ARR(mFramesData[i]);
	}
}

float AnimationTrack::mix(float time, int segmentNdx, int targetComponentIndex)
{
	float mixFactor = 0;
	vec2 Pm1;
	vec2 P0;
	vec2 P1;
	vec2 C0;
	vec2 C1;
	vec2 Pp2;

	//prepare values
	//perform calculation
	switch(mInterpolationType)
	{
	case LINEAR:
		mixFactor = (time - mTimeline[segmentNdx]) / (mTimeline[segmentNdx+1] - mTimeline[segmentNdx]);//TODO: optimize - move this calculation outside
		break;

	case CARDINAL:
	case BSPLINE:
		{
			int minus1Ndx	= segmentNdx > 0 ? segmentNdx - 1 : segmentNdx;
			int plus2Ndx	= segmentNdx+2 < (int)mTimeline.size() ? segmentNdx + 2 : segmentNdx + 1;

			Pm1.x = mTimeline[minus1Ndx];
			Pp2.x = mTimeline[plus2Ndx];

			Pm1.y = mFramesData[minus1Ndx][0];
			Pp2.y = mFramesData[plus2Ndx][0];

			//TODO: implement calculation of mixFactor
		}

	case HERMITE:
		//TODO: implement correct calculation of mixFactor
	case BEZIER:
		//P0 = POSITION[segmentNdx]
		//P1 = POSITION[segmentNdx + 1]
		P0.x = mTimeline[segmentNdx];
		P1.x = mTimeline[segmentNdx + 1];

		P0.y = mFramesData[segmentNdx][targetComponentIndex];
		P1.y = mFramesData[segmentNdx + 1][targetComponentIndex];

		if(mInterpolationType == CARDINAL || mInterpolationType == BSPLINE)
			break;

		//C0 = OUT_TANGENTS[segmentNdx]
		C0.x = mTangentsData[(segmentNdx * 2) + 0].y;
		C0.y = mTangentsData[(segmentNdx * 2) + 1].y;

		//C1 = IN_TANGENTS[segmentNdx + 1]
		C1.x = mTangentsData[(segmentNdx * 2) + 2].x;
		C1.y = mTangentsData[(segmentNdx * 2) + 3].x;

		mixFactor = ApproximateCubicBezierParameter(time, P0.x, C0.x, C1.x, P1.x);

		break;
	default: break;
	}

	//perform calculation
	switch(mInterpolationType)
	{
	case STEP:
#ifdef ACCURATE_STEP_ANIMATION
		return ((time - mTimeline[segmentNdx]) < (mTimeline[segmentNdx + 1] - time)) ? mFramesData[segmentNdx][targetComponentIndex] : mFramesData[segmentNdx + 1][targetComponentIndex];
#else
		return mFramesData[segmentNdx][targetComponentIndex];
#endif
	case LINEAR:
		return mixValue(mFramesData[segmentNdx][targetComponentIndex], mFramesData[segmentNdx+1][targetComponentIndex], mixFactor);
	case BEZIER:
		return InterpolateCubicBezier(P0.y, C0.y, C1.y, P1.y, mixFactor);
	case HERMITE:
		return InterpolateCubicHermite(P0.y, C0.y, C1.y, P1.y, mixFactor);
	case CARDINAL:
		return InterpolateCardinal(Pm1.y, P0.y, P1.y, Pp2.y, mixFactor, 0.5f);
	case BSPLINE:
		return InterpolateBSpline(Pm1.y, P0.y, P1.y, Pp2.y, mixFactor);
	default:
		return mFramesData[segmentNdx][targetComponentIndex];
		break;
	}

	return mFramesData[segmentNdx][targetComponentIndex];
}

int AnimationTrack::defineSegment(float time)
{
	const size_t segmentsNum	= mTimeline.size()-1;
	//find keyframes closest to time value
	for(int i = 0; i < segmentsNum; ++i)
	{
		if(time >= mTimeline[i] && time <= mTimeline[i+1])
		{
			return i;
		}
	}
	return -1;
}

void AnimationTrack::updateTarget(float time)
{
	ASSERT(mTarget);
	updateBuffer(time, mTarget, NULL);
}

void AnimationTrack::updateBuffer(float time, float * buffer, bool ** changeFlags)
{
	const size_t targetCompsNum	= mTargetIndices.size();
	const size_t framesNum			= mFramesData.size();

	ASSERT(buffer);
	ASSERT(framesNum > 0);
	ASSERT(targetCompsNum > 0);
	ASSERT(framesNum == mTimeline.size());

	int i = 0;

	int lowerBoundIndex = -1;

	//find keyframes closest to time value
	lowerBoundIndex = defineSegment(time);

	//do not update target if time is out of timeline
	if(lowerBoundIndex < 0)
	{
		return;
	}

	//float * lowerFrameData = mFramesData[lowerBoundIndex];
	//float * upperFrameData = mFramesData[lowerBoundIndex+1];

	//update target
	//now supports only one targetCompsNum if erpType is not Linear or Step
	for(i = 0; i < targetCompsNum; ++i)
	{
		buffer[ mTargetIndices[i] ] = this->mix(time, lowerBoundIndex, i); //this->mix(lowerFrameData[i], upperFrameData[i], mixValue);
		if(changeFlags != NULL)
		{
			(*changeFlags[ mTargetIndices[i] ]) = true;
		}
	}
}

void AnimationTrack::convertToLinear(float fps)
{
	convertTo(LINEAR, fps);
}

void AnimationTrack::convertToStep(float fps)
{
	convertTo(STEP, fps);	
}

//now supports only linear and setep
void AnimationTrack::convertTo(InterpolationType targetErpType, float fps)
{
	if(targetErpType != LINEAR && targetErpType != STEP)
	{
		//now supports only linear and setep
		return;
	}
	if(mInterpolationType == STEP)
	{
		mInterpolationType = targetErpType;
		return;//need no conversions
	}
	if(mInterpolationType == LINEAR && targetErpType == LINEAR)
	{
		return;//need no conversions
	}

	const size_t targetCompsNum	= mTargetIndices.size();

	float trackTime = mTimeline.back() - mTimeline.front();
	float aproxFramesNum = trackTime*fps;
	int newFramesNum = (int)ceil( aproxFramesNum );
	float frameTime = trackTime/(newFramesNum - 1);//divide by segments num (instead of frames num)

	FLOAT_ARR			newTimeline(newFramesNum);
	PFLOAT_ARR			newFramesData(newFramesNum);

	//build new timeline and frames data
	float time = mTimeline.front();
	for(int i = 0; i < newFramesNum; ++i)
	{
		newFramesData[i] = new float[targetCompsNum];

		int lowerBoundIndex = defineSegment(time);

		//now supports only one targetCompsNum if erpType is not Linear or Step
		for(int j = 0; j < targetCompsNum; ++j)
		{
			newFramesData[i][j] = this->mix(time, lowerBoundIndex, j); //this->mix(lowerFrameData[i], upperFrameData[i], mixValue);
		}

		newTimeline[i] = time;
		time = clamp(time + frameTime, mTimeline.front(), mTimeline.back());
		if(i==newFramesNum - 2) time = mTimeline.back();
	}

	mInterpolationType = targetErpType;

	//replace new data with old

	//replace timeline
	mTimeline.clear();
	mTimeline.resize(newFramesNum);
	mTimeline.assign(newTimeline.begin(), newTimeline.end());

	//replace frames
	for(uint32 i = 0; i < mFramesData.size(); ++i)
	{
		DELETE_ARR(mFramesData[i]);
	}
	mFramesData.clear();
	mFramesData.resize(newFramesNum);
	mFramesData.assign(newFramesData.begin(), newFramesData.end());

	//remove unnecessary data

	mTangentsData.clear();
}

}//namespace Resource { 

}//namespace Squirrel {