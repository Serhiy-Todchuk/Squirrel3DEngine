#pragma once

#include <Math/mathTypes.h>
#include <vector>
#include "macros.h"

#ifdef	_WIN32
//	disable warning on extern before template instantiation
#	pragma warning( disable: 4231 )
#endif

namespace Squirrel {

namespace Resource { 

//#define ACCURATE_STEP_ANIMATION

//TODO: to make Spline or Curve class

//animates one or more float values
class SQRESOURCE_API AnimationTrack
{
public://nested types

	enum InterpolationType
	{
		UNKNOWN = 0,
		STEP,
		LINEAR,
		BEZIER,
		HERMITE,//->TCB; not tested
		CARDINAL,//not tested
		BSPLINE,//not tested
		MIXED//TODO implement
	};

	/*
	SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::vector<float>;
	SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::vector<float *>;
	SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::vector<int>;
	SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::vector<Math::vec2>;
	*/

	typedef std::vector<float>		FLOAT_ARR;
	typedef std::vector<float *>	PFLOAT_ARR;
	typedef std::vector<int>		INT_ARR;
	typedef std::vector<Math::vec2>	VEC2_ARR;

private://members

	InterpolationType	mInterpolationType;

	FLOAT_ARR			mTimeline;
	PFLOAT_ARR			mFramesData;
	VEC2_ARR			mTangentsData;

	float *				mTarget;
	INT_ARR				mTargetIndices;

	//mTimeline.size()		== framesNum
	//mFramesData.size()	== framesNum
	//mTangentsData.size()	== ~framesNum
	//mTargetIndices.size()	== targetComponentsNum
	//mFramesData[n].size()	== targetComponentsNum
	//mTimeline.front()		< mTimeline.end()

public:
	AnimationTrack(float * target, int targetComponentsNum, int framesNum);
	virtual ~AnimationTrack(void);

	void updateTarget(float time);//deprecated
	void updateBuffer(float time, float * buffer, bool ** changeFlags);

	void convertToLinear(float fps);
	void convertToStep(float fps);

	//accessors

	void	setTarget(float *target)	{ mTarget = target; }
	float *	getTarget()					{ return mTarget; }

	void setInterpolationType(InterpolationType interpolationType)	{ mInterpolationType = interpolationType; }
	InterpolationType	getInterpolationType()	{ return mInterpolationType; }
	
	FLOAT_ARR *		getTimeline()		{ return &mTimeline; }
	PFLOAT_ARR *	getFramesData()		{ return &mFramesData; }
	INT_ARR *		getTargetIndices()	{ return &mTargetIndices; }
	VEC2_ARR *		getTangentsData()	{ return &mTangentsData; }

	size_t			getTargetComponentsNum() { return mTargetIndices.size(); };

private:

	float mix(float time, int segmentNdx, int targetComponentIndex);
	void convertTo(InterpolationType targetErpType, float fps);
	int defineSegment(float time);
};


}//namespace Resource { 

}//namespace Squirrel {