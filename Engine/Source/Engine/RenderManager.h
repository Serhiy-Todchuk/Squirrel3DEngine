#pragma once

#include <World/World.h>
#include <Resource/Mesh.h>
#include <Render/IRender.h>
#include <Render/IFrameBuffer.h>
#include <Render/Light.h>
#include <Render/Camera.h>
#include <Resource/Program.h>
#include "PostFXManager.h"
#include "Shadow.h"

namespace Squirrel {
namespace Engine {

using namespace Render;

class SQENGINE_API RenderManager: 
	public DepthRenderer
{
private:

	Resource::Mesh * mQuadMesh; 

	bool mClearColor;

	std::string mShadowsType;
	bool mEnableShadows;
	
	int mParallaxSteps;
	float mParallaxDistance;
	
	float mFogDistance;

	std::auto_ptr<ShadowsManager> mShadowsManager;

	PostFXManager mPostFXManager;

	std::auto_ptr<ITexture> mColorBufferTwin;
	std::auto_ptr<ITexture> mDepthBufferTwin;

	struct Reflection
	{
		mat4 matrix;
		std::auto_ptr<IFrameBuffer> buffer;
	};

	typedef std::map<ReflectionDesc, std::auto_ptr<Reflection> > REFLECTIONS_MAP;

	REFLECTIONS_MAP mReflections;

	Render::RenderQueue mMainRenderQueue;
	Render::RenderQueue mReflRenderQueue;

	World::RenderInfo mMainPassRenderOptions;
	World::RenderInfo mSecondaryLitPassRenderOptions;
	World::RenderInfo mDepthRenderOptions;
	World::RenderInfo mReflectionRenderOptions;

	UniformContainer mUniformsPool;

public:
	RenderManager();
	virtual ~RenderManager();

	virtual void init();
	virtual void render(World::World * world);
	virtual void begin();
	virtual void end();

	void draw2DDebugInfo(float screenWidth, float screenHeight);
	void draw3DDebugInfo();

	void setClearColor(bool flag) { mClearColor = flag; }
	bool getClearColor() { return mClearColor; }

private:

	enum RenderLightPassFlags {
		rlpShadows = 1,
		rlpMainPass = 2,
	};

	static const int sDefaultLightPassFlags = rlpShadows | rlpMainPass;

	void renderPostFX(Camera * cam);
	void buildShadows(World::World * world);
	void render(Render::RenderQueue& renderQueue, Camera * cam,
				const World::RenderInfo& info,
				int flags = sDefaultLightPassFlags);

	void createShadows(Render::Light * light, World::World * world);
	void setQuadSize(float x, float y);

	void renderLit(Render::RenderQueue * renderQueue, Render::Light * light,
				   const World::RenderInfo& info,
				   int flags = sDefaultLightPassFlags);

	//implement DepthRenderer
	void renderDepthOnly(Render::RenderQueue * renderQueue, const std::string& params = "" );
	const World::RenderInfo& getDepthRenderOptions() { return mDepthRenderOptions; }
};

}//namespace Engine { 
}//namespace Squirrel {