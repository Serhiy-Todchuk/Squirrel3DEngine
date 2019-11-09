#pragma once

#include <World/World.h>
#include <Resource/Mesh.h>
#include <Render/IRender.h>
#include <Render/IFrameBuffer.h>
#include <Render/Light.h>
#include <Render/Camera.h>
#include <Resource/Program.h>
#include <Common/Settings.h>
#include "macros.h"

namespace Squirrel {
namespace Engine { 

class Shadow;

class DepthRenderer
{
public:
	virtual ~DepthRenderer() {}

	void setEyePos(vec4 eyePos) { mEyePos = eyePos; }
	virtual void renderDepthOnly(Render::RenderQueue * renderQueue, const std::string& params = "" ) = 0;

	virtual const World::RenderInfo& getDepthRenderOptions() = 0;
	
protected:
	vec4 mEyePos;
};

class ShadowsManager
{
public:
	static const int MAX_DIR_SHADOWS	= 2;
	static const int MAX_SPOT_SHADOWS	= 16;
	static const int MAX_OMNI_SHADOWS	= 8;

	ShadowsManager(Settings * settings, const char_t * sectionName);
	~ShadowsManager() {}

	void clear();

	Shadow * getShadow(Render::Light * light);
	Shadow * addShadow(Render::Light * light);

private:

	std::map<Render::Light *, int> mShadowsTable;

	std::auto_ptr<Shadow> mDirShadows[MAX_DIR_SHADOWS];
	std::auto_ptr<Shadow> mSpotShadows[MAX_SPOT_SHADOWS];
	std::auto_ptr<Shadow> mOmniShadows[MAX_OMNI_SHADOWS];

	int mDirShadowsNum;
	int mSpotShadowsNum;
	int mOmniShadowsNum;

	int mDirShadowSize;
	int mOmniShadowSize;
	int mSpotShadowSize;

	int mShadowSplitsNum;

	float mPCFOffset;

	vec3 mSplitDistances;
};

class Shadow
{
public:
	Shadow(): light(NULL), framebuffer(NULL), mapSize(256), mPCFOffset(2.0f) {}
	virtual ~Shadow() { 		
		DELETE_PTR(framebuffer);
	}

	virtual bool build(World::World * world, DepthRenderer * renderer) = 0; 
	virtual void bind(Render::IProgram * program) = 0; 

	void setMapSize(int mapSize_) { mapSize = mapSize_; }
	void setLight(Render::Light * light_) { light = light_; }

	const std::string& getProgramParams() const { return mProgramParams; }

protected:
	Render::Light			* light;
	Render::IFrameBuffer	* framebuffer;
	int						  mapSize;
	float					  mPCFOffset;

	std::string mProgramParams;

	Render::RenderQueue mRenderQueue;
};

class OneMapShadow: 
	public Shadow
{
public:
	OneMapShadow(): camera(NULL), map(NULL) {}
	virtual ~OneMapShadow() {
		DELETE_PTR(camera);
		DELETE_PTR(map);
	}
	
	Render::ITexture		* getMap() { return map; }

protected:
	Render::Camera			* camera;
	Render::ITexture		* map;
};

class SpotShadow: 
	public OneMapShadow
{
public:
	SpotShadow() {}
	virtual ~SpotShadow() {}

	virtual bool build(World::World * world, DepthRenderer * renderer);
	virtual void bind(Render::IProgram * program);
};

class OmniShadow: 
	public OneMapShadow
{
public:
	OmniShadow() {}
	virtual ~OmniShadow() {}

	virtual bool build(World::World * world, DepthRenderer * renderer);
	virtual void bind(Render::IProgram * program);
};

class DirectionalShadow: 
	public Shadow
{
public:
	static const int MAX_SHADOW_SPLITS	= 4;

public:
	DirectionalShadow(): splitsNum(1)
	{
		memset(splitCameras, 0, sizeof(void *) * MAX_SHADOW_SPLITS);
		memset(splitMaps, 0, sizeof(void *) * MAX_SHADOW_SPLITS);
	}
	virtual ~DirectionalShadow() 	{
		for(int i = 0; i < MAX_SHADOW_SPLITS; ++i)
		{
			DELETE_PTR(splitCameras[i]);
			DELETE_PTR(splitMaps[i]);
		}
	}

	virtual bool build(World::World * world, DepthRenderer * renderer);
	virtual void bind(Render::IProgram * program);

	void setSplitsNum(int splitsNum_) {
		splitsNum = splitsNum_;
	}
	void setSplitsDistances(vec3 splitDistances_) {
		splitDistances = splitDistances_;
	}

protected:

	int						splitsNum;

	vec3					splitDistances;

	vec4					mPCFOffsets;

	Render::Camera			* splitCameras[MAX_SHADOW_SPLITS];
	Render::ITexture		* splitMaps[MAX_SHADOW_SPLITS];
};

}//namespace Engine { 
}//namespace Squirrel {