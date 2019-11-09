#include "DynamicSkySphere.h"
#include <Common/TimeCounter.h>

namespace Squirrel {
namespace Engine { 

using namespace Math;
using namespace Resource;

SQREFL_REGISTER_CLASS(Engine::DynamicSkySphere);

DynamicSkySphere::DynamicSkySphere(): mMesh(NULL), mProgram(NULL)
{
	SQREFL_SET_CLASS(Engine::DynamicSkySphere);

	mTopColor		= vec4(0.7f, 0.9f, 0.9f, 1);
	mBottomColor	= vec4(0.1f, 0.1f, 0.3f, 0.1f);

	mBgrRotation = quat();
	mBgrRotationSpeed = 0.1f;

	mTimeFlowSpeed = 0.f;
	wrapAtomicField("TimeFlowSpeed", &mTimeFlowSpeed);
}

DynamicSkySphere::~DynamicSkySphere() 
{
	DELETE_PTR(mBgrSkybox);
	DELETE_PTR(mMesh);
	if(mProgram != NULL)
		ProgramStorage::Active()->release(mProgram->getID());
}

DynamicSkySphere::Orb& DynamicSkySphere::addSun()
{
	mOrbs.push_back(Orb());
	Orb& orb = mOrbs.back();

	orb.mAxis	= vec3(0, 0, 1);
	orb.mPos	= vec3(0, 1, 0);
	orb.mSecondsPerTurn = 40;

	orb.mRadius	= 0.005f;
	orb.mInfluence	= 0.8f;
	orb.mIntensity	= 1;

	orb.mTopColor		= vec4(1.0f, 0.95f, 0.75f, 1);
	orb.mBottomColor	= vec4(0.9f, 0.5f, 0.7f, 1);

	orb.mCurrentAngle = PI * 0.08f;

	return orb;
}

void DynamicSkySphere::init(StaticSkyBox * backgroundSkybox)
{
	using namespace RenderData;

	mBgrSkybox = backgroundSkybox;

	const float boxSize = 10;

	SphereBuilder * meshBuilder = new SphereBuilder( boxSize, 8, 8, VT_P );

	mMesh = meshBuilder->buildMesh();

	DELETE_PTR(meshBuilder);

	mMesh->getVertexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mMesh->getIndexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mMesh->getIndexBuffer()->setPolyOri(IndexBuffer::poClockWise);

	mProgram = Resource::ProgramStorage::Active()->add("sky/DynamicSkySphere.glsl");
}

void DynamicSkySphere::init(const char_t * backgroundSkyboxTextureName)
{
	Engine::StaticSkyBox * backgroundSkybox = new Engine::StaticSkyBox();
	backgroundSkybox->init("skyboxes/purpleNebulaStars2.sqtex");
	init(backgroundSkybox);
}

void DynamicSkySphere::render(Render::IRender * render, Render::Camera * camera)
{
	ASSERT(mMesh != NULL);

	mat4 skyTransform = mat4().identity();

	if(camera != NULL)
		skyTransform.setTranslate(camera->getPosition());

	if(render == NULL)
	{
		render = Render::IRender::GetActive();
		ASSERT(render != NULL);
	}

	render->setBlendMode(Render::IRender::blendOneMinusAlpha);
	render->setAlphaTestValue(0.0f);

	if(mBgrSkybox != NULL)
	{
		mBgrSkybox->render(render, camera);
	}

	render->setTransform(skyTransform);

	Render::IProgram * program = mProgram->getRenderProgram("");

	program->bind();
	program->uniform("skyTopColor", mTopColor.getVec3());
	program->uniform("skyBottomColor", mBottomColor.getVec3());

	Orb& orb = mOrbs.front();

	program->uniform("orbColor", orb.mColor);
	program->uniform("orbDir", orb.mPos);
	program->uniform("orbRadius", orb.mRadius);
	
	//compute fog color based on code from shader which calculates sky color
	
	vec3 orbDir = orb.mPos;
	
	vec3 n;
	n.getProjectPlane(vec3(0, 1, 0), 0, orbDir);
	n.normalize();
	
	float orbDotProduct = 2.0 - (1.0 + (n * orbDir));	//0..2, 0 - orb center, 1 - half distant from orb, 2 - farest point from orb
	float orbInvHeight = - (orbDir.y - 1.0);	//0..2, 0 - zenit, 1 - horizont, 2 - bottom
	float skyInvHeight = - (n.y - 1.0);		//0..2, 0 - zenit, 1 - horizont, 2 - bottom
	
	skyInvHeight = lerp(skyInvHeight, orbDotProduct, orbInvHeight * 0.5);
	
	float skyInvShininess = skyInvHeight * 0.5 + orbInvHeight * 0.5;
	vec3 skyBgrColor = lerp(mTopColor.getVec3(), mBottomColor.getVec3(), skyInvShininess);
	
	/*
	float r = minValue(2.0 - orbInvHeight, 1.0) * 0.5;
	float orbAtt = maxValue(1.0-(orbDotProduct*orbDotProduct/(r*r)),0.0);
	vec3 skyColor = lerp(skyBgrColor, orb.mColor, orbAtt * orbAtt);
	*/

	mHorizontColor = vec4(skyBgrColor, 1);

	render->getUniformsPool().fetchUniforms(program);

	render->setupVertexBuffer( mMesh->getVertexBuffer() );

	render->enableDepthTest(Render::IRender::depthTestOff);
	render->enableDepthWrite(false);

	render->renderIndexBuffer( mMesh->getIndexBuffer() );

	render->enableDepthTest();
	render->enableDepthWrite(true);
}

void DynamicSkySphere::update()
{
	if(mOrbs.size() == 0)
		return;

	float dtime = TimeCounter::Instance().getDeltaTime();

	Orb& orb = mOrbs.front();

	float radiansPerSecond = TWO_PI * (1.0f/orb.mSecondsPerTurn);
	
	orb.mCurrentAngle += dtime * radiansPerSecond * mTimeFlowSpeed;

	float angleDiff = orb.mCurrentAngle - TWO_PI;

	if(angleDiff >= 0)
	{
		orb.mCurrentAngle = angleDiff;
	}

	vec3 orbDir;
	quat rot;

	rot.fromAxisAngle(orb.mAxis, orb.mCurrentAngle);
	orbDir = vec3(0, 1, 0) ^ orb.mAxis;

	orb.mPos = rot.apply(orbDir).normalized(); 

	float orbInvHeight = - (orb.mPos.y - 1);	//0..2, 0 - zenit, 1 - horizont, 2 - bottom
	orb.mColor = lerp(orb.mTopColor, orb.mBottomColor, orbInvHeight * 0.5f).getVec3();

	if(mLight)
	{
		mLight->mLightType = (Light::ltDirectional);
		mLight->mDiffuse = vec4ToColorBytes(vec4(orb.mColor, 1.0f));
		mLight->mSpecular = vec4ToColorBytes(vec4(orb.mColor, 1.0f));
		mLight->setDirection(orb.mPos);
	}

	quat bgrRotIncr;
	bgrRotIncr.fromAxisAngle(vec3(1,-1,-1).normalized(), dtime * mBgrRotationSpeed);
	mBgrRotation = mBgrRotation & bgrRotIncr;
	mBgrSkybox->setRotation(mBgrRotation.toRotationMatrix());
}

}//namespace Engine { 
}//namespace Squirrel {
