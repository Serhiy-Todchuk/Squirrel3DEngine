#include "Shadow.h"

namespace Squirrel {
namespace Engine { 

using namespace Render;

UniformString sUniformShadowMatrix			("uShadowMatrix");
UniformString sUniformShadowMap				("shadowMap");
UniformString sUniformShadowMap1			("shadowMapX");
UniformString sUniformShadowMap2			("shadowMapXX");
UniformString sUniformShadowMap3			("shadowMapXXX");
UniformString sUniformShadowMatrices		("uShadowMatrices");
UniformString sUniformShadowMaps			("shadowMaps");
UniformString sUniformShadowOffset			("shadowOffset");
UniformString sUniformShadowSplitDistances	("shadowSplitDistances");

mat4 shadowBiasMatrix(	0.5f, 0.0f, 0.0f, 0.5f, 
						0.0f, 0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 0.5f, 0.5f,
						0.0f, 0.0f, 0.0f, 1.0f	);

ShadowsManager::ShadowsManager(Settings * settings, const char_t * sectionName)
{
	mDirShadowsNum	= 0;
	mSpotShadowsNum	= 0;
	mOmniShadowsNum	= 0;

	mDirShadowSize	= settings->getInt(sectionName, "Dir Shadow Size", 1024);
	mOmniShadowSize	= settings->getInt(sectionName, "Omni Shadow Size", 1024);
	mSpotShadowSize	= settings->getInt(sectionName, "Spot Shadow Size", 1024);

	mShadowSplitsNum = Settings::Default()->getInt(sectionName, "Shadow Splits Num", 3);

	mSplitDistances.x	= settings->getFloat(sectionName, "First Split Distance", 32.0f);
	mSplitDistances.y	= settings->getFloat(sectionName, "Second Split Distance", 128.0f);
	mSplitDistances.z	= settings->getFloat(sectionName, "Third Split Distance", 256.0f);
}

void ShadowsManager::clear()
{
	int i = 0;
	for(i = 0; i < mDirShadowsNum; ++i)
		mDirShadows[i].release();
	for(i = 0; i < mSpotShadowsNum; ++i)
		mSpotShadows[i].release();
	for(i = 0; i < mOmniShadowsNum; ++i)
		mOmniShadows[i].release();

	mDirShadowsNum	= 0;
	mSpotShadowsNum	= 0;
	mOmniShadowsNum	= 0;
}

Shadow * ShadowsManager::addShadow(Light * light)
{
	DirectionalShadow * dirShadow;
	OmniShadow * omniShadow;
	SpotShadow * spotShadow;

	int index = 0;
	switch(light->mLightType)
	{
	case Light::ltDirectional:
		if(mDirShadowsNum == MAX_DIR_SHADOWS)
			return NULL;

		index = mDirShadowsNum;
		++mDirShadowsNum;

		mShadowsTable[light] = index;

		dirShadow = new DirectionalShadow();
		mDirShadows[index].reset(dirShadow);

		dirShadow->setMapSize(mDirShadowSize);
		dirShadow->setSplitsNum(mShadowSplitsNum);
		dirShadow->setSplitsDistances(mSplitDistances);

		return dirShadow;

	case Light::ltOmni:
		if(mOmniShadowsNum == MAX_OMNI_SHADOWS)
			return NULL;

		index = mOmniShadowsNum;
		++mOmniShadowsNum;

		mShadowsTable[light] = index;

		omniShadow = new OmniShadow;
		mOmniShadows[index].reset(omniShadow);

		omniShadow->setMapSize(mOmniShadowSize);

		return omniShadow;

	case Light::ltSpot:
		if(mSpotShadowsNum == MAX_SPOT_SHADOWS)
			return NULL;

		index = mSpotShadowsNum;
		++mSpotShadowsNum;

		mShadowsTable[light] = index;

		spotShadow = new SpotShadow;
		mSpotShadows[index].reset(spotShadow);

		spotShadow->setMapSize(mSpotShadowSize);

		return spotShadow;
	case Light::ltUnknown:
		ASSERT(false);
	};

	return NULL;
}

Shadow * ShadowsManager::getShadow(Light * light)
{
	std::map<Light *, int>::iterator it = mShadowsTable.find(light);
	if(it == mShadowsTable.end())
	{
		return NULL;
	}

	int index = it->second;

	if(index < 0)
		return NULL;

	switch(light->mLightType)
	{
	case Light::ltDirectional:
		if(index < mDirShadowsNum)
			return mDirShadows[index].get();
		break;
	case Light::ltOmni:
		if(index < mOmniShadowsNum)
			return mOmniShadows[index].get();
		break;
	case Light::ltSpot:
		if(index < mSpotShadowsNum)
			return mSpotShadows[index].get();
		break;
	case Light::ltUnknown:
		ASSERT(false);
	};

	return NULL;
}

bool SpotShadow::build(World::World * world, DepthRenderer * renderer)
{
	IRender * render = IRender::GetActive();

	if(!camera)
	{
		camera = new Render::Camera(Camera::Perspective);
	}

	if(!map)
	{
		map = render->createTexture();
		map->generate();
		map->fill(ITexture::pfDepth32, tuple3i(mapSize, mapSize, 1));
		map->enableShadow(true);
	}

	if(!framebuffer)
	{
		framebuffer = render->createFrameBuffer(mapSize, mapSize, 0);
		framebuffer->generate();
		framebuffer->create();
		framebuffer->attachDepthTexture(map);
		framebuffer->isOk();
	}

	//setup camera

	const float nearPlane= 0.1f;
	
	vec3 lightDir = light->getDirection().normalized();
	vec3 right = lightDir ^ ((lightDir * vec3(0,1,0)) < 0.9f ? vec3(0,1,0) : vec3(1, 0, 0));
	vec3 up = right ^ lightDir;
	
	camera->setPosition( light->getPosition() );
	camera->setDirection( lightDir );
	camera->setUp(up);
	camera->buildProjection(light->mOuterSpotAngle, 1.0f, nearPlane, light->mRadius + nearPlane);

	//render into depth map

	framebuffer->bind();

	mRenderQueue.clear();

	world->renderRecursively(&mRenderQueue, camera, renderer->getDepthRenderOptions());

	render->setProjection( camera->getFinalMatrix() );

	renderer->renderDepthOnly(&mRenderQueue);

	framebuffer->unbind();

	mProgramParams = "";

	return true;
}

void SpotShadow::bind(Render::IProgram * program)
{
	program->uniform(sUniformShadowOffset, mPCFOffset/mapSize );

	mat4 shadowProjMatrix = shadowBiasMatrix * camera->getFinalMatrix();
	program->uniform(sUniformShadowMatrix, shadowProjMatrix);

	int shadowMapUnit = program->getSamplerUnit(sUniformShadowMap);
	if(shadowMapUnit >= 0)
		map->bind(shadowMapUnit);
}

bool OmniShadow::build(World::World * world, DepthRenderer * renderer)
{
	IRender * render = IRender::GetActive();

	if(!camera)
	{
		camera = new Render::Camera(Camera::Perspective);
	}

	if(!map)
	{
		map = render->createTexture();
		map->generate();
		map->fillCube(ITexture::pfDepth32, mapSize);
	}

	if(!framebuffer)
	{
		framebuffer = render->createFrameBuffer(mapSize, mapSize, 0);
		framebuffer->generate();
		framebuffer->create();
	}

	//setup camera

	const float nearPlane= 0.1f;

	camera->setPosition( light->getPosition() );
	camera->buildProjection(HALF_PI, 1.0f, nearPlane, light->mRadius + nearPlane);

	const vec3 cubeMapCameraDirections[ITexture::cmfNum]=	
	{
		vec3( 1, 0, 0),
		vec3(-1, 0, 0),
		vec3( 0,-1, 0),
		vec3( 0, 1, 0),
		vec3( 0, 0,-1),
		vec3( 0, 0, 1)
	};

	const vec3 cubeMapCameraUpVecs[ITexture::cmfNum]=
	{
		vec3( 0, 1, 0),
		vec3( 0, 1, 0),
		vec3( 0, 0,-1),
		vec3( 0, 0, 1),
		vec3( 0, 1, 0),
		vec3( 0, 1, 0)
	};

	framebuffer->bind();

	for(int i = 0; i < ITexture::cmfNum; ++i)
	{
		camera->setDirection( cubeMapCameraDirections[i] );
		camera->setUp( cubeMapCameraUpVecs[i] );
		camera->update();

		framebuffer->attachDepthTextureFace(map, i);
		framebuffer->isOk();

		//render into depth map

		mRenderQueue.clear();
		
		world->renderRecursively(&mRenderQueue, camera, renderer->getDepthRenderOptions());

		render->setProjection( camera->getFinalMatrix() );

		renderer->setEyePos(vec4(light->getPosition(), light->mRadius));
		renderer->renderDepthOnly(&mRenderQueue, "WRITE_DISTANCE;");
	}

	framebuffer->unbind();

	mProgramParams = "";

	return true;
}

void OmniShadow::bind(Render::IProgram * program)
{
	program->uniform(sUniformShadowOffset, mPCFOffset / mapSize );

	int shadowMapUnit = program->getSamplerUnit(sUniformShadowMap);
	if(shadowMapUnit >= 0)
		map->bind(shadowMapUnit);
}

void setupShadowCameraForViewCamera(Camera * viewCamera, Camera * shadowCamera, World::World * world, vec3 lightDir)
{
	AABB worldBounds = world->getVisibleBounds();

	vec3 right = lightDir ^ vec3(0,1,0);
	vec3 up = right ^ lightDir;

	//extract frustum points, clamp them with world bounds and find center of the group of points

	vec3 frustumPoints[Camera::FRUSTUM_POINTS];
	vec3 center(0, 0, 0);

	int i;

	for(i = 0; i < Camera::FRUSTUM_POINTS; ++i)
	{
		vec3 pt = viewCamera->getPoint(i);

		pt = worldBounds.clampPoint( pt );

		center += pt;

		frustumPoints[i] = pt;
	}

	center /= Camera::FRUSTUM_POINTS;

	//find bounds of shadow in light space

	mat4 shadowLookAtMatrix = Camera::CalcLookAtMatrix(vec3::Zero(), -lightDir, up);

	AABB shadowVolumeBounds;

	for(i = 0; i < Camera::FRUSTUM_POINTS; ++i)
	{
		vec3 pt = frustumPoints[i];

		pt -= center;

		pt = shadowLookAtMatrix * pt;

		shadowVolumeBounds.addVertex(pt);
	}

	//calc camera position

	vec3 shadowCamPos = center - lightDir * shadowVolumeBounds.min.z;
	
	//setup shadow camera 

	const float nearPlaneDist = 0.1f;
	float farPlaneDist = shadowVolumeBounds.max.z - shadowVolumeBounds.min.z + nearPlaneDist;

	shadowCamera->setPosition( shadowCamPos );
	shadowCamera->setDirection( - lightDir );
	shadowCamera->setUp(up);
	shadowCamera->buildProjection(shadowVolumeBounds.min.x, shadowVolumeBounds.max.x, shadowVolumeBounds.min.y, shadowVolumeBounds.max.y, nearPlaneDist, farPlaneDist);
}

bool DirectionalShadow::build(World::World * world, DepthRenderer * renderer)
{
	//get main camera
	Camera * mainCam = Camera::GetMainCamera();

	IRender * render = IRender::GetActive();

	if(mainCam == NULL)
		return false;

	vec3 lightDir = light->getDirection().normalized();

	std::auto_ptr<Camera> viewCam(new Camera(*mainCam));

	if(!framebuffer)
	{
		framebuffer = render->createFrameBuffer(mapSize, mapSize, 0);
		framebuffer->generate();
		framebuffer->create();
		//framebuffer->isOk();
	}

	framebuffer->bind();

	float viewNearPlane	= viewCam->getNear();
	float viewFarPlane	= viewCam->getFar();

	float nearPlane	= viewNearPlane;
	float farPlane	= viewFarPlane;

	mPCFOffsets[0] = mPCFOffset / mapSize;

	for(int i = 0; i < splitsNum; ++i)
	{
		if(!splitCameras[i])
			splitCameras[i] = new Render::Camera(Camera::Orthographic);

		float prevProjectionSize = farPlane - nearPlane;

		//calc near plane distance
		if(i == 0)
			nearPlane = viewNearPlane;
		else
			nearPlane = splitDistances[i - 1] * 0.95f;

		//calc far plane distance
		if(i == splitsNum - 1)
			farPlane = viewFarPlane;
		else
			farPlane = splitDistances[i];

		if(i > 0)
		{
			float projectionSize = farPlane - nearPlane;
			mPCFOffsets[i] = mPCFOffsets[i - 1];// * (prevProjectionSize / projectionSize);
		}

		//
		viewCam->buildProjection(viewCam->getFov(), viewCam->getAspect(), nearPlane, farPlane);

		setupShadowCameraForViewCamera(viewCam.get(), splitCameras[i], world, lightDir);

		if(!splitMaps[i])
		{
			splitMaps[i] = render->createTexture();
			splitMaps[i]->generate();
			splitMaps[i]->fill(ITexture::pfDepth32, tuple3i(mapSize, mapSize, 1));
			splitMaps[i]->enableShadow(true);
		}
	
		framebuffer->attachDepthTexture(splitMaps[i]);
		
		mRenderQueue.clear();

		world->renderRecursively(&mRenderQueue, splitCameras[i], renderer->getDepthRenderOptions());

		render->setProjection( splitCameras[i]->getFinalMatrix() );

		renderer->renderDepthOnly(&mRenderQueue);
	}

	framebuffer->unbind();

	if(splitsNum > 1)
		mProgramParams = MakeString() << "SHADOW_SPLITS " << splitsNum << ";";
	else
		mProgramParams = "";

	return true;
}

void DirectionalShadow::bind(Render::IProgram * program)
{
	if(splitsNum == 1)
	{
		program->uniform(sUniformShadowOffset, mPCFOffsets[0] );

		mat4 shadowProjMatrix = shadowBiasMatrix * splitCameras[0]->getFinalMatrix();
		program->uniform(sUniformShadowMatrix, shadowProjMatrix);
		int shadowMapUnit = program->getSamplerUnit(sUniformShadowMap);
		if(shadowMapUnit >= 0)
			splitMaps[0]->bind(shadowMapUnit);
	}
	else
	{
		program->uniform(sUniformShadowOffset, mPCFOffsets );

		program->uniform(sUniformShadowSplitDistances, splitDistances);
		
		mat4 shadowProjMatrices[MAX_SHADOW_SPLITS];
		for(int i = 0; i < splitsNum; ++i)
			shadowProjMatrices[i] = shadowBiasMatrix * splitCameras[i]->getFinalMatrix();

		program->uniformArray(sUniformShadowMatrices, splitsNum, shadowProjMatrices);

		int unit = program->getSamplerUnit(sUniformShadowMap);
		if(unit >= 0)
			splitMaps[0]->bind( unit );
		unit = program->getSamplerUnit(sUniformShadowMap1);
		if(splitsNum > 1 && unit >= 0)
			splitMaps[1]->bind( unit );
		unit = program->getSamplerUnit(sUniformShadowMap2);
		if(splitsNum > 2 && unit >= 0)
			splitMaps[2]->bind( unit );
		unit = program->getSamplerUnit(sUniformShadowMap3);
		if(splitsNum > 3 && unit >= 0)
			splitMaps[3]->bind( unit );
	}
}

}//namespace Engine { 
}//namespace Squirrel {
