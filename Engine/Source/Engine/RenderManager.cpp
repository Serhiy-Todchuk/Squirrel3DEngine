#include "RenderManager.h"
#include <Resource/ProgramStorage.h>
#include <Render/IProgram.h>
#include <Render/Camera.h>
#include <Render/Light.h>
#include <Render/IFrameBuffer.h>
#include <Render/VertexBuffer.h>
#include <Resource/Mesh.h>
#include <Common/TimeCounter.h>
#include <Common/Settings.h>

namespace Squirrel {
namespace Engine { 

using namespace RenderData;
using namespace Render;

#define RENDERING_SETTINGS_SECTION "Rendering"

UniformString sUniformLightDir			("lightDir");
UniformString sUniformAmbient			("ambient");
UniformString sUniformDiffuse			("diffuse");
UniformString sUniformSpecular			("specular");
UniformString sUniformShininess			("shininess");
UniformString sUniformLightPos			("lightPos");
UniformString sUniformLightRad			("lightRad");
UniformString sUniformLightInnerCone	("lightInnerCone");
UniformString sUniformLightOuterCone	("lightOuterCone");
UniformString sUniformFogColor			("fogColor");
UniformString sUniformFogEnd			("fogEnd");
UniformString sUniformFogStart			("fogStart");
UniformString sUniformBones				("bones");
UniformString sUniformEyePos			("eyePos");
UniformString sUniformParallaxSteps		("uParallaxSteps");
UniformString sUniformParallaxScale		("uParallaxScale");
UniformString sUniformReflectionMatrix	("uReflectionMatrix");
UniformString sUniformReflectionMap		("uReflectionMap");
UniformString sUniformSceneColorMap		("uSceneColorMap");
UniformString sUniformSceneDepthMap		("uSceneDepthMap");
UniformString sUniformClipPlane			("uClipPlane");

World::World * sWorld = NULL;

Resource::Program * programBumpy = 0;
Resource::Program * programBuildShadow = 0;
Resource::Program * programRenderCubeMap = 0;
Resource::Program * programRenderShadowMap = 0;
	
Render::ITexture * litRampTexture = 0;

int timeNodeCollectBatches = 0;
int timeNodeBuildShadows = 0;
int timeNodeRenderWorld = 0;
int timeNodeRenderTransparent = 0;

const int MAX_DEBUG_FRUSTUMS_NUM = 2;
Camera * debugFrustums[MAX_DEBUG_FRUSTUMS_NUM] = { NULL, NULL };

ITexture * debugCubemap = NULL;
ITexture * debugShadowMap = NULL;
	
RenderManager::RenderManager() { mClearColor = true; };
RenderManager::~RenderManager() {};

void RenderManager::setQuadSize(float x, float y)
{
	VertexBuffer * vb = mQuadMesh->getVertexBuffer();
	vb->setComponent<VertexBuffer::vcPosition>(0, vec3(0, 0, 0));
	vb->setComponent<VertexBuffer::vcPosition>(1, vec3(0, y, 0));
	vb->setComponent<VertexBuffer::vcPosition>(2, vec3(x, y, 0));
	vb->setComponent<VertexBuffer::vcPosition>(3, vec3(x, 0, 0));
}

void drawDebugFrustums()
{
	IRender * render = IRender::GetActive();
	
	VertexBuffer * vb = render->createVertexBuffer(VT_P, Camera::FRUSTUM_POINTS);
	vb->setStorageType(IBuffer::stCPUMemory);
	IndexBuffer * ib = render->createIndexBuffer(24, IndexBuffer::Index16);
	ib->setPolyType(IndexBuffer::ptLines);
	ib->setStorageType(IBuffer::stCPUMemory);
	
	int index = 0;
	
	ib->setIndex(index++, 0);
	ib->setIndex(index++, 1);
	ib->setIndex(index++, 1);
	ib->setIndex(index++, 2);
	ib->setIndex(index++, 2);
	ib->setIndex(index++, 3);
	ib->setIndex(index++, 3);
	ib->setIndex(index++, 0);
	
	ib->setIndex(index++, 4);
	ib->setIndex(index++, 5);
	ib->setIndex(index++, 5);
	ib->setIndex(index++, 6);
	ib->setIndex(index++, 6);
	ib->setIndex(index++, 7);
	ib->setIndex(index++, 7);
	ib->setIndex(index++, 4);
	
	ib->setIndex(index++, 0);
	ib->setIndex(index++, 4);
	ib->setIndex(index++, 1);
	ib->setIndex(index++, 5);
	ib->setIndex(index++, 2);
	ib->setIndex(index++, 6);
	ib->setIndex(index++, 3);
	ib->setIndex(index++, 7);
	
	vec4 colors[MAX_DEBUG_FRUSTUMS_NUM] = { vec4(0, 1, 0, 1), vec4(1, 0, 0, 1) };
	
	for(int i = 0; i < MAX_DEBUG_FRUSTUMS_NUM; ++i)
	{
		if(debugFrustums[i] == NULL)
			continue;
		
		for(int j = 0; j < Camera::FRUSTUM_POINTS; ++j)
			vb->setComponent<VertexBuffer::vcPosition>(j, debugFrustums[i]->getPoint(j));
		
		IProgram * program = IProgram::GetBoundProgram();
		if(program)
		{
			render->setColor(colors[i]);
			render->getUniformsPool().fetchUniforms(program);
		}
		
		render->setupVertexBuffer(vb);
		render->renderIndexBuffer(ib);
	}
	
	DELETE_PTR(vb);
	DELETE_PTR(ib);
		
}

void RenderManager::draw3DDebugInfo()
{
	return;

	drawDebugFrustums();
}
	
void RenderManager::draw2DDebugInfo(float screenWidth, float screenHeight)
{
	return;

	if(debugCubemap)
	{
		IRender * render = IRender::GetActive();
		
		const float radius = 50;
		
		Resource::SphereBuilder sphBuilder(radius, 16, 16, VT_P);
		Resource::Mesh * mesh = sphBuilder.buildMesh();
		
		IProgram * program = programRenderCubeMap->getRenderProgram("");
		
		debugCubemap->bind();
		
		program->bind();
		render->getUniformsPool().fetchUniforms(program);
		
		mat4 tform = mat4::Translate(vec3(screenWidth - radius, screenHeight - radius, 0));
		render->setTransform( tform, program );
		
		render->setupVertexBuffer(mesh->getVertexBuffer());
		render->renderIndexBuffer(mesh->getIndexBuffer());
		
		DELETE_PTR(mesh);
	}
	else if(debugShadowMap)
	{
		IRender * render = IRender::GetActive();
		
		const float width = 50;
		
		setQuadSize(width, width);
		IProgram * program = programRenderShadowMap->getRenderProgram("");
		
		debugShadowMap->bind();
		
		program->bind();
		render->getUniformsPool().fetchUniforms(program);
		
		mat4 tform = mat4::Transform(vec3(screenWidth - width, screenHeight - width, 0), vec3(1, 1, 1));
		render->setTransform( tform, program );
		
		render->setupVertexBuffer(mQuadMesh->getVertexBuffer());
		render->renderIndexBuffer(mQuadMesh->getIndexBuffer());
	}

}

void RenderManager::init()
{
	Resource::ProgramStorage * programStorage = Resource::ProgramStorage::Active();

	programBumpy			= programStorage->add("forward/bumpy.glsl");
	programBuildShadow		= programStorage->add("forward/buildShadowMap.glsl");
	programRenderCubeMap	= programStorage->add("renderCubeMap.glsl");
	programRenderShadowMap	= programStorage->add("renderShadowMap.glsl");
	
	Resource::TextureStorage * texStorage = Resource::TextureStorage::Active();
	Resource::Texture * lLitRampTexture = texStorage->loadTexture("lit_ramp.jpg");
	if(lLitRampTexture)
		litRampTexture = lLitRampTexture->getRenderTexture();
	
	mQuadMesh = new Resource::Mesh();
	IndexBuffer * ib = mQuadMesh->createIndexBuffer(6);
	ib->setIndex(0, 0);
	ib->setIndex(1, 1);
	ib->setIndex(2, 2);
	ib->setIndex(3, 0);
	ib->setIndex(4, 2);
	ib->setIndex(5, 3);

	VertexBuffer * vb = mQuadMesh->createVertexBuffer(VT_PT, 4);
	vb->setComponent<VertexBuffer::vcTexcoord>(0, vec2(0, 0));
	vb->setComponent<VertexBuffer::vcTexcoord>(1, vec2(0, 1));
	vb->setComponent<VertexBuffer::vcTexcoord>(2, vec2(1, 1));
	vb->setComponent<VertexBuffer::vcTexcoord>(3, vec2(1, 0));
	setQuadSize(200, 200);

	mShadowsManager.reset(new ShadowsManager(Settings::Default(), RENDERING_SETTINGS_SECTION));
	mPostFXManager.init(Settings::Default());

	mEnableShadows	= Settings::Default()->getInt(RENDERING_SETTINGS_SECTION, "EnableShadows", 1) != 0;
	mShadowsType	= Settings::Default()->getString(RENDERING_SETTINGS_SECTION, "ShadowsType", "SHADOW_PCF_5TAP");
	mParallaxSteps	= Settings::Default()->getInt(RENDERING_SETTINGS_SECTION, "ParallaxMappingSteps", 16);
	mParallaxDistance	= Settings::Default()->getFloat(RENDERING_SETTINGS_SECTION, "ParallaxMappingDistance", 16.0f);

	//init tmp benchmark
	timeNodeCollectBatches		= TimeCounter::Instance().addNode("  collectBatches");
	timeNodeBuildShadows		= TimeCounter::Instance().addNode("  buildShadows");
	timeNodeRenderWorld			= TimeCounter::Instance().addNode("  renderWorld");
	timeNodeRenderTransparent	= TimeCounter::Instance().addNode("  renderFX");

	mMainPassRenderOptions.layersRange			= tuple2i(0, rqMax);
	mMainPassRenderOptions.level				= World::rilLighting;

	mSecondaryLitPassRenderOptions.layersRange	= tuple2i(rqBackground + 1, rqTransparent - 1);
	mSecondaryLitPassRenderOptions.level		= World::rilLighting;

	mDepthRenderOptions.layersRange				= tuple2i(rqBackground + 1, rqTransparent - 1);
	mDepthRenderOptions.level					= World::sBuildShadowRILevel;

	mReflectionRenderOptions.layersRange		= tuple2i(0, rqOverlay - 1);
	mReflectionRenderOptions.minLOD				= sLODGoodForReflections;
	mReflectionRenderOptions.level				= World::rilLighting;
}

void RenderManager::createShadows(Light * light, World::World * world)
{
	Shadow * shadow = mShadowsManager->getShadow(light);

	if(shadow == NULL)
		shadow = mShadowsManager->addShadow(light);

	if(shadow == NULL)
		return;

	shadow->setLight(light);

	shadow->build(world, this);
	
	if(light->mLightType == Light::ltOmni)
	{
		debugCubemap = ((OmniShadow *)shadow)->getMap();
	}
	if(light->mLightType == Light::ltSpot)
	{
		debugShadowMap = ((SpotShadow *)shadow)->getMap();
	}
}

void RenderManager::render(World::World * world)
{
	TimeCounter::Instance().setNodeTimeBegin(timeNodeCollectBatches);

	if(world == NULL)
		return;

	sWorld = world;

	//get main camera
	Render::Camera * cam = Render::Camera::GetMainCamera();

	if(cam == NULL)
		return;

	mMainRenderQueue.clear();
	world->renderRecursively(&mMainRenderQueue, cam, mMainPassRenderOptions);

	Render::IRender * render = Render::IRender::GetActive();

	render->getRenderStatistics().mBatchesNum = 0;

	TimeCounter::Instance().setNodeTimeEnd(timeNodeCollectBatches);
	TimeCounter::Instance().setNodeTimeBegin(timeNodeBuildShadows);

	render->setAlphaTestValue(0.5f);

	if(mEnableShadows)
	{
		for(std::list<Light*>::iterator itLight = mMainRenderQueue.getLights().begin();
			itLight != mMainRenderQueue.getLights().end(); ++itLight)
		{
			if((*itLight)->mShadow)
				createShadows((*itLight), world);
		}
	}

	TimeCounter::Instance().setNodeTimeEnd(timeNodeBuildShadows);
	TimeCounter::Instance().setNodeTimeBegin(timeNodeRenderWorld);

	render->setAlphaTestValue(0.5f);

	//render reflections

	FOREACH(RenderQueue::REFL_DESCS_SET::iterator, itReflDesc, mMainRenderQueue.getRequiredReflections())
	{
		Reflection * refl = NULL;
		REFLECTIONS_MAP::iterator itRefl = mReflections.find(*itReflDesc);
		if(itRefl == mReflections.end())
		{
			refl = new Reflection;
			mReflections[*itReflDesc] = std::auto_ptr<Reflection>(refl);
		}
		else
		{
			refl = itRefl->second.get();
		}

		const int reflMapSize = 512;

		if(!refl->buffer.get())
		{
			refl->buffer.reset( render->createFrameBuffer(reflMapSize, reflMapSize, IFrameBuffer::depth16) );
			refl->buffer->generate();
			refl->buffer->create();

			ITexture * tex = render->createTexture();
			tex->generate();
			tex->fill(ITexture::pfColor4Byte, tuple3i(reflMapSize, reflMapSize, 1));
			refl->buffer->attachColorTexture(tex);
		}

		refl->buffer->bind();

		Render::Camera reflCam(*cam);
		reflCam.setPosition(reflCam.getPosition().mul(vec3(1, -1, 1)));
		reflCam.setDirection(reflCam.getDirection().mul(vec3(1, -1, 1)));
		reflCam.setUp(vec3(0, -1, 0));
		reflCam.update();

		mReflRenderQueue.clear();

		vec4 reflClipPlane(0, 1, 0, 0.5f);
		mReflectionRenderOptions.clipPlane = &reflClipPlane;

		world->renderRecursively(&mReflRenderQueue, &reflCam, mReflectionRenderOptions);


		this->render(mReflRenderQueue, &reflCam, mReflectionRenderOptions, 0);

		mat4 reflMatrixBias(0.5f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.5f, 0.0f, 0.5f,
							0.0f, 0.0f, 0.5f, 0.5f,
							0.0f, 0.0f, 0.0f, 1.0f	);

		refl->matrix = reflMatrixBias * reflCam.getFinalMatrix();
	}
	
	//render final scene

	if(mPostFXManager.getSceneFrameBuffer() != NULL)
		mPostFXManager.getSceneFrameBuffer()->bind();
	else
	{
		IFrameBuffer::Unbind();
		tuple2i screen = render->getWindow()->getSize();
		render->setViewport(0,0,screen.x,screen.y);
	}
	
	this->render(mMainRenderQueue, cam, mMainPassRenderOptions, rlpShadows);

	//render water

	TimeCounter::Instance().setNodeTimeEnd(timeNodeRenderWorld);
	TimeCounter::Instance().setNodeTimeBegin(timeNodeRenderTransparent);

	world->renderCustomRecursively(render, cam, mMainPassRenderOptions);

	renderPostFX(cam);

	TimeCounter::Instance().setNodeTimeEnd(timeNodeRenderTransparent);
}

void RenderManager::renderPostFX(Camera * cam)
{
	Render::IRender * render = Render::IRender::GetActive();

	float time = TimeCounter::Instance().getTime();
	float dtime = TimeCounter::Instance().getDeltaTime();
	tuple2i screenSizeI = render->getWindow()->getSize();
	vec2 screenSize = vec2(screenSizeI.x, screenSizeI.y);
	float nearDist = cam->getNear();
	float farDist = cam->getFar();
	vec3 projectedOrbPos;//TODO: implement (for sunshafts)

	mPostFXManager.getPostFXUniforms()->uniformArray("uTime", 1, &time);
	mPostFXManager.getPostFXUniforms()->uniformArray("uDeltaTime", 1, &dtime);
	mPostFXManager.getPostFXUniforms()->uniformArray("uScreenSize", 1, &screenSize);
	mPostFXManager.getPostFXUniforms()->uniformArray("uCamNear", 1, &nearDist);
	mPostFXManager.getPostFXUniforms()->uniformArray("uCamFar", 1, &farDist);
	mPostFXManager.getPostFXUniforms()->uniformArray("uScreenSize", 1, &screenSize);
	mPostFXManager.getPostFXUniforms()->uniformArray("uProjOrbPos", 1, &projectedOrbPos);

	Render::Utils::Begin2D();
	mPostFXManager.process();
	Render::Utils::End2D();
}
	
void RenderManager::render(Render::RenderQueue& renderQueue, Camera * cam, const World::RenderInfo& info, int flags)
{
	Render::IRender * render = Render::IRender::GetActive();

	render->setProjection(cam->getFinalMatrix());

	float fogStart = sWorld->getEffectiveViewDistance() * 0.65f;
	mUniformsPool.uniformArray(sUniformFogStart, 1, &fogStart);

	float fogEnd = sWorld->getEffectiveViewDistance();
	mUniformsPool.uniformArray(sUniformFogEnd, 1, &fogEnd);

	vec3 eyePos = cam->getPosition();
	mUniformsPool.uniformArray(sUniformEyePos, 1, &eyePos);

	vec2 uNearFarClips(cam->getNear(), cam->getFar());
	mUniformsPool.uniformArray("uNearFarClips", 1, &uNearFarClips);

	float time(TimeCounter::Instance().getTime());
	mUniformsPool.uniformArray("uTime", 1, &time);

	if(sWorld->getSky() != NULL)
	{
		render->setColor( Math::vec4(1.4f, 0.1f, 1.4f, 1.0f) );
		render->clear(false, true);//do not clear color buffer when render sky
		sWorld->getSky()->render(render, cam);
		vec4 fogColor = sWorld->getSky()->getHorizontColor();
		render->getUniformsPool().uniformArray(sUniformFogColor, 1, &fogColor);
	}
	else
	{
		render->clear(true, true);
		vec4 fogColor = vec4(0.4f, 0.5f, 0.7f, 1);
		render->getUniformsPool().uniformArray(sUniformFogColor, 1, &fogColor);
	}

	bool firstLight = true;

	render->setAlphaTestValue(0.5f);
	
	for(std::list<Light*>::iterator itLight = renderQueue.getLights().begin();
		itLight != renderQueue.getLights().end(); ++itLight)
	{
		int lightPassFlags = flags;

		if(	firstLight )
		{
			render->setBlendMode(IRender::blendOff);
			lightPassFlags |= rlpMainPass;
			firstLight = false;
		}
		else
		{
			render->setBlendMode(IRender::blendAdd);

			vec4 fogColor = vec4(0, 0, 0, 1);
			render->getUniformsPool().uniformArray(sUniformFogColor, 1, &fogColor);
		}

		renderLit(&renderQueue, (*itLight), info, lightPassFlags);
	}
}

void RenderManager::renderLit(RenderQueue * renderQueue, Light * light, const World::RenderInfo& info, int flags)
{
	IRender * render = IRender::GetActive();

	Shadow * shadow = mShadowsManager->getShadow(light);
	if(!light->mShadow)
		shadow = NULL;
	if(!(flags & rlpShadows))
		shadow = NULL;

	render->enableDepthTest();
	render->enableDepthWrite(true);
	render->enablePolygonOffset(false);

	std::stringstream passProgramParams;

	if(info.clipPlane)
	{
		passProgramParams << "CLIP;";
	}

	passProgramParams << "LIT_PHONG;";

	if(light->mLightType == Light::ltOmni)
		passProgramParams << "POINT_LIGHT;";
	else if(light->mLightType == Light::ltSpot)
		passProgramParams << "SPOT_LIGHT;";
	else
		passProgramParams << "DIR_LIGHT;";

	if(shadow)
	{
		passProgramParams << mShadowsType << ";SHADOW_MAP;" << shadow->getProgramParams();
	}

	MaterialGroup *		currentMatGroup			= NULL;
	VBGroup *			currentVBGroup			= NULL;

	IProgram * program = NULL;

	RenderQueue::RENDER_OPS_LIST * renderOps = renderQueue->getRenderOpsList();
	FOREACH(RenderQueue::RENDER_OPS_LIST::iterator, itRenderOp, (*renderOps))
	{
		const RenderOp& renderOp = (*itRenderOp);

		if(!light->doesLit(renderOp.mIndexPrimitive->mAABB))
			continue;

		if(currentMatGroup != renderOp.mMaterialGroup)
		{
			if(!(rlpMainPass & flags) && renderOp.mMaterialGroup->mRenderOnce)
				continue;

			//don't render reflective materials into reflections
			if(renderOp.mMaterialGroup->mRequiresReflection && info.clipPlane)
				continue;

			MaterialGroup *	prevMatGroup	= currentMatGroup;
			currentMatGroup					= renderOp.mMaterialGroup;

			bool programSwitched = false;

			if(prevMatGroup == NULL || !currentMatGroup->sameProgram(*prevMatGroup))
			{
				std::stringstream programParams;

				programParams << passProgramParams.str();
				programParams << currentMatGroup->mProgramParams.str();

				//load program

				program = NULL;

				if(currentMatGroup->mProgramName.length() > 0)
				{
					Resource::ProgramStorage * programStorage = Resource::ProgramStorage::Active();
					Resource::Program * programResource	= programStorage->add(currentMatGroup->mProgramName.str());
					if(programResource)
					{
						program = programResource->getRenderProgram(programParams.str());
					}
				}

				if(program == NULL)
				{
					program = programBumpy->getRenderProgram(programParams.str());
				}

				programSwitched = true;

				program->bind();
				
				if(shadow != NULL)
				{
					shadow->bind(program);
				}

				switch(light->mLightType)
				{
					case Render::Light::ltSpot:
					{
						float innerCone = light->mInnerSpotAngle * DEG2RAD * 0.5f;
						float outerCone = light->mOuterSpotAngle * DEG2RAD * 0.5f;
						if(innerCone >= outerCone)
						{
							float tmp = innerCone;
							innerCone = outerCone;
							outerCone = tmp;
						}
						if(Math::equals(outerCone, innerCone))
						{
							innerCone = outerCone - 0.001f;
						}
						program->uniform(sUniformLightInnerCone, Math::fcos(innerCone) );
						program->uniform(sUniformLightOuterCone, Math::fcos(outerCone) );
					}
					case Render::Light::ltOmni:
						program->uniform(sUniformLightPos, light->getPosition() );
						program->uniform(sUniformLightRad, light->mRadius );
						break;
					default:
						break;
				}

				/*
				 if(parallaxMapping)
				 {
				 program->uniform(sUniformParallaxSteps, (float)mParallaxSteps);
				 program->uniform(sUniformParallaxScale, currentMatGroup->mReliefScale);
				 }
				 */

				if(info.clipPlane)
				{
					program->uniform(sUniformClipPlane, (*info.clipPlane));
				}

				mUniformsPool.fetchUniforms(program);
			}

			if(programSwitched || prevMatGroup == NULL || currentMatGroup->mMaterial != prevMatGroup->mMaterial)
			{
				//default material settings
				vec3 diffuse	= vec3(0.8f, 0.8f, 0.8f);
				vec3 specular	= vec3(0.4f, 0.33f, 0.25f);
				vec3 ambient	= vec3(0.4f, 0.33f, 0.25f);
				float shininess = 32.0f;

				if(currentMatGroup->mMaterial)
				{
					ambient		= currentMatGroup->mMaterial->mAmbient.getVec3();
					diffuse		= currentMatGroup->mMaterial->mDiffuse.getVec3();
					specular	= currentMatGroup->mMaterial->mSpecular.getVec3();
					shininess	= currentMatGroup->mMaterial->mShininess;
				}

				//if(hasSpecularMap) specular = vec3::One();

				diffuse = diffuse.mul( light->getDiffuse().getVec3() );
				specular = specular.mul( light->getSpecular().getVec3() );
				ambient = ambient.mul( light->getAmbient().getVec3() );
				vec3 lightDir = light->getDirection();

				program->uniform(sUniformLightDir, lightDir );
				program->uniform(sUniformAmbient, ambient );
				program->uniform(sUniformDiffuse, diffuse );
				program->uniform(sUniformSpecular, vec4(specular, shininess) );
				program->uniform(sUniformShininess, shininess );
			}

			if(programSwitched || prevMatGroup == NULL || !currentMatGroup->sameTextures(*prevMatGroup))
			{
				//bind textures
				FOREACH(MaterialGroup::TEXTURES_MAP::const_iterator, itTexture, currentMatGroup->mTextures)
				{
					if(itTexture->second != NULL)
					{
						int unit = program->getSamplerUnit(itTexture->first);
						if(unit >= 0)
						{
							itTexture->second->bind(unit);
						}
					}
				}
			}

			if(currentMatGroup->mRequiresReflection)
			{
				REFLECTIONS_MAP::iterator itRefl = mReflections.find(currentMatGroup->mReflectionDesc);
				if(itRefl != mReflections.end())
				{
					Reflection * refl = itRefl->second.get();

					program->uniform(sUniformReflectionMatrix, refl->matrix);

					int mapUnit = program->getSamplerUnit(sUniformReflectionMap);
					if(mapUnit >= 0)
					{
						refl->buffer->getAttachement()->bind(mapUnit);
					}
				}
			}

			if(currentMatGroup->mRequiresColorBuffer)
			{
				int mapUnit = program->getSamplerUnit(sUniformSceneColorMap);
				if(mapUnit >= 0)
				{
					tuple2i screenSize = render->getWindow()->getSize();

					if(mColorBufferTwin.get() == NULL)
					{
						mColorBufferTwin.reset(render->createTexture());
						mColorBufferTwin->generate();
					}

					mColorBufferTwin->bind(mapUnit);
					mColorBufferTwin->fillFromScreen(ITexture::pfColor4Byte, screenSize.x, screenSize.y);
				}
			}
			
			if(currentMatGroup->mRequiresDepthBuffer)
			{
				int mapUnit = program->getSamplerUnit(sUniformSceneDepthMap);
				if(mapUnit >= 0)
				{
					tuple2i screenSize = render->getWindow()->getSize();

					if(mDepthBufferTwin.get() == NULL)
					{
						mDepthBufferTwin.reset(render->createTexture());
						mDepthBufferTwin->generate();
					}

					mDepthBufferTwin->bind(mapUnit);
					mDepthBufferTwin->fillFromScreen(ITexture::pfDepth24, screenSize.x, screenSize.y);
				}
			}

			//if(currentMatGroup->mSemiTransparent)
			{
				//render->setBlendMode(IRender::blendOneMinusAlpha);
				//render->setAlphaTestValue(0.0f);
			}
			//else
			{
				//render->setBlendMode(IRender::blendOff);
				//render->setAlphaTestValue(0.001f);
			}
			
			currentMatGroup->mUniformsPool.fetchUniforms(program);

			render->getUniformsPool().fetchUniforms(program);
		}

		if(currentVBGroup != renderOp.mVBGroup)
		{
			currentVBGroup = renderOp.mVBGroup;
			
			render->setupVertexBuffer( currentVBGroup->mVB );

			if(currentVBGroup->mBonesCount > 0)
			{
				program->uniformArray(sUniformBones, currentVBGroup->mBonesCount, currentVBGroup->mBonesData);
			}
		}

		FOREACH(IndexPrimitive::TRANSFORMS_ARRAY::iterator, itInstance, renderOp.mIndexPrimitive->mTransforms)
		{
			const mat4& transform = (*itInstance);

			render->setTransform( transform, program );

			render->renderIndexBuffer( renderOp.mIndexPrimitive->mIB );
			
			++render->getRenderStatistics().mBatchesNum;
		}
	}
}

void RenderManager::renderDepthOnly(RenderQueue * renderQueue, const std::string& params)
{
	Render::IRender * render = Render::IRender::GetActive();

	render->enableColorWrite(false);

	render->clear(false, true);

	render->enableDepthTest();
	render->enablePolygonOffset(true, 8, 800);

	Render::IProgram * program = NULL;

	MaterialGroup *		currentMatGroup			= NULL;
	VBGroup *			currentVBGroup			= NULL;

	std::string passProgramParams = "TEXTURE_ALPHA;";

	RenderQueue::RENDER_OPS_LIST * renderOps = renderQueue->getRenderOpsList();
	FOREACH(RenderQueue::RENDER_OPS_LIST::iterator, itRenderOp, (*renderOps))
	{
		const RenderOp& renderOp = (*itRenderOp);

		if(currentMatGroup != renderOp.mMaterialGroup)
		{
			MaterialGroup *	prevMatGroup	= currentMatGroup;
			currentMatGroup					= renderOp.mMaterialGroup;
			
			bool programSwitched = false;

			if(prevMatGroup == NULL || !currentMatGroup->sameProgram(*prevMatGroup))
			{
				program = programBuildShadow->getRenderProgram(passProgramParams + currentMatGroup->mProgramParams.str());

				programSwitched = true;

				if(program)
				{
					program->bind();
				}
				
				//mUniformsPool.fetchUniforms(program);

				program->uniform("lightPos", mEyePos);
			}

			if(programSwitched || prevMatGroup == NULL || !currentMatGroup->sameTextures(*prevMatGroup))
			{
				//bind textures
				FOREACH(MaterialGroup::TEXTURES_MAP::const_iterator, itTexture, currentMatGroup->mTextures)
				{
					if(itTexture->second != NULL)
					{
						int unit = program->getSamplerUnit(itTexture->first);
						if(unit >= 0)
						{
							itTexture->second->bind(unit);
						}
					}
				}
			}

			render->getUniformsPool().fetchUniforms(program);
		}

		if(currentVBGroup != renderOp.mVBGroup)
		{
			currentVBGroup = renderOp.mVBGroup;

			render->setupVertexBuffer( currentVBGroup->mVB );

			if(currentVBGroup->mBonesCount > 0 && program)
			{
				program->uniformArray(sUniformBones, currentVBGroup->mBonesCount, currentVBGroup->mBonesData);
			}
		}

		FOREACH(IndexPrimitive::TRANSFORMS_ARRAY::iterator, itInstance, renderOp.mIndexPrimitive->mTransforms)
		{
			render->setTransform( (*itInstance), program );

			//swap cull face
			int cullFace = renderOp.mIndexPrimitive->mIB->getPolyOri();
			if(cullFace == IndexBuffer::poCounterClockWise)
				cullFace = IndexBuffer::poClockWise;
			else if(cullFace == IndexBuffer::poClockWise)
				cullFace = IndexBuffer::poCounterClockWise;

			render->renderIndexBuffer( renderOp.mIndexPrimitive->mIB );

			++render->getRenderStatistics().mBatchesNum;
		}
	}

	render->enablePolygonOffset(false);

	render->enableColorWrite(true);
}

void RenderManager::begin()
{
	Render::IRender * render = Render::IRender::GetActive();
	render->setBlendMode(Render::IRender::blendOff);
	render->setAlphaTestValue(0.5f);
	render->setColor(Math::vec4(1,1,1,1));
	render->setTransform(Math::mat4().identity());
}

void RenderManager::end()
{
	Render::IRender * render = Render::IRender::GetActive();
	render->present();
	
	render->setColor(Math::vec4(0,0.5f,0,1));
	render->clear(mClearColor, true);
}

}//namespace Engine { 
}//namespace Squirrel {
