#pragma once

#include <Common/common.h>
#include <Common/Log.h>
#include <Common/TimeCounter.h>
#include <Render/IRenderable.h>
#include <Render/Light.h>
#include <Resource/ResourceManager.h>
#include <Resource/Mesh.h>
#include <World/Behaviour.h>
#include <World/World.h>
#include <list>
#include "StaticSkyBox.h"
#include "macros.h"

namespace Squirrel {
namespace Engine { 

class SQENGINE_API DynamicSkySphere:
	public World::Behaviour,
	public World::Sky
{
public:
	struct Orb
	{
		Orb() {};
		~Orb() {};

		Math::vec4 mTopColor;
		Math::vec4 mBottomColor;

		float mRadius;
		float mIntensity;
		float mInfluence;

		Math::vec3 mPos;
		Math::vec3 mAxis;
		float mSecondsPerTurn;

		Resource::Texture * mTexture;

		float mCurrentAngle;
		Math::vec3 mColor;
	};

private:

	StaticSkyBox *	mBgrSkybox;
	Math::quat		mBgrRotation;
	float			mBgrRotationSpeed;

	RenderData::Light *	mLight;

	Resource::Mesh * mMesh;
	Resource::Program * mProgram;

	Math::vec4 mTopColor;
	Math::vec4 mBottomColor;

	vec4 mHorizontColor;

	float mTimeFlowSpeed;

	std::list<Orb> mOrbs;

	virtual void update();

public:
	DynamicSkySphere();
	~DynamicSkySphere();

	Orb& addSun();

	void setLight(RenderData::Light * light) { mLight = light; }
	void init(StaticSkyBox * backgroundSkybox);
	void init(const char_t * backgroundSkyboxTextureName);

	virtual void render(Render::IRender * render, Render::Camera * camera);

	virtual vec4 getHorizontColor() { return mHorizontColor; }
};

}//namespace Engine { 
}//namespace Squirrel {