#pragma once

#include <common/common.h>
#include <Common/Data.h>
#include <Resource/Mesh.h>
#include <Resource/ResourceManager.h>
#include "SceneObject.h"
#include <list>

namespace Squirrel {
namespace World { 

using namespace Math;

class SQWORLD_API ParticleSystem: 
	public SceneObject
{
public:

	static const int MAX_ANIM_COLORS = 5;

	enum RenderWay
	{
		rwSoftwareBillboards = 0,
		rwBillboards,
		rwSoftBillboards,
		rwPointSprites
		//rwGPUBuffer
	};

	struct Particle
	{
		vec3 pos;
		vec3 velocity;
		vec4 color;
		float size;
		float angularVelocity;
		float angle;
		float energy;
		float initialEnergy;
	};

	ParticleSystem();
	//ParticleSystem(): ParticleSystem(MAX_PARTICLES_NUM) {}//waiting for C++0x
	virtual ~ParticleSystem();

	void init(const std::string& textureName, int maxParticlesNum = 5000, RenderWay renderWay = rwBillboards);

public:

	virtual void renderCustom(Render::IRender * render, Render::Camera * camera, const RenderInfo& info);
	virtual void update(float dtime);

	virtual void deserialize(Reflection::Deserializer * deserializer);

	void setEmit(bool e);

	void setSizeRange(vec2 minMax) { mSizeRange = minMax; }
	void setEnergyRange(vec2 minMax) { mEnergyRange = minMax; }
	void setEmissionRange(vec2 minMax) { mEmissionRange = minMax; }

	void setStartVelocity(vec3 vec) { mStartVelocity = vec; }
	void setStartRndVelocity(vec3 vec) { mStartRndVelocity = vec; }

	void setEmissionElipsoid(vec3 sz) { mEmissionElipsoid = sz; }

	void setSizeGrow(float sizeGrow) { mSizeGrow = sizeGrow; }

	void setAnimColor(int index, tuple4ub color) { mAnimColors[index] = color; }

	void setRotation(bool flag) { mRotation = flag; }
	void setStartRotation(float degreesPerSecond) { mStartRotation = degreesPerSecond; }
	void setStartRndRotation(float degreesPerSecond) { mStartRndRotation = degreesPerSecond; }

protected:

	virtual void calcAABB();

private:

	void emitParticle();

	void onTextureNameChaned();

private:
	//particles buffer

	std::list<Particle> mParticles;
	int mMaxParticlesNum;

	//emission params

	bool mEmit;

	vec2 mSizeRange;//particle size (min and max)
	vec2 mEnergyRange;//longivity of particle's life in seconds (min and max)
	vec2 mEmissionRange;//number of particles per second (min and max)

	vec3 mStartVelocity;
	vec3 mStartRndVelocity;

	bool mRotation;
	float mStartRotation;
	float mStartRndRotation;

	vec3 mEmissionElipsoid;

	//anim params

	bool mAnimateColors;
	int	mAnimColorsNum;
	tuple4ub mAnimColors[MAX_ANIM_COLORS];

	float mSizeGrow;

	vec3 mForce;
	vec3 mRandomForce;

	float mDumping;

	//render params

	RenderWay mRenderWay;
	Resource::Mesh * mMesh;
	Resource::Texture * mTexture;
	Resource::Program * mProgram;

	float mTimeLeftFromLastUpdate;

	std::string mTextureName;

};


}//namespace World { 
}//namespace Squirrel {
