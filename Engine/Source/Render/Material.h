#pragma once

#include <Common/Types.h>
#include <Math/vec4.h>
#include <string>
#include "macros.h"

namespace Squirrel {

namespace RenderData { 

class SQRENDER_API Material
{
public:
	Material(void);
	~Material(void);

	//getters
	const std::string&	getName	()	const		{return mName;}
	_ID			getID()	const					{ return mID;}

	//setters
	void		setName	(const std::string& name)		{mName	= name;}
	void		setID(_ID id)					{ mID	= id;}

private:

	_ID		mID;

	std::string	mName;

public:
	
	std::string mLighting;
	std::string mPhysics;

	Math::vec4	mDiffuse;
	Math::vec4	mSpecular;
	Math::vec4	mEmission;
	Math::vec4	mAmbient;
	Math::vec4	mReflect;
	float		mShininess;
	float		mOpacity;
	float		mRoughness;
	float		mAnisotropy;
	float		mReliefScale;
	float		mSSSCoef;
	Math::vec4	mReserved[4];
};


}//namespace RenderData { 

}//namespace Squirrel {