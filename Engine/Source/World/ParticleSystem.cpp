// ParticleSystem.cpp: implementation of the ParticleSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "ParticleSystem.h"
#include <Resource/TextureStorage.h>
#include <Render/IProgram.h>
#include <Reflection/AtomicWrapper.h>
#include <Reflection/CollectionWrapper.h>
#include <Reflection/EnumWrapper.h>

namespace Squirrel {
namespace World { 

#define MAX_PARTICLES_NUM	8192

SQREFL_REGISTER_CLASS_SEED(World::ParticleSystem, WorldParticleSystem);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParticleSystem::ParticleSystem():
	mProgram(NULL), mMesh(NULL), mTexture(NULL), mMaxParticlesNum(MAX_PARTICLES_NUM)
{
	//init emission params

	mEmit = true;

	mSizeRange			= vec2(1.1f, 3.0f);//particle size (min and max)
	mEnergyRange		= vec2(6.1f, 13.0f);//longivity of particle's life in seconds (min and max)
	mEmissionRange		= vec2(20.0f, 20.0f);//number of particles per second (min and max)

	mStartVelocity		= vec3(0, 2, 0);
	mStartRndVelocity	= vec3(0.2f, 1, 0.2f);

	mRotation			= true;
	mStartRotation		= -90;
	mStartRndRotation	= 180;

	mEmissionElipsoid	= vec3(2, 1, 2);

	//anim members

	mAnimateColors = true;
	mAnimColors[0] = tuple4ub(255, 255, 255, 0);
	mAnimColors[1] = tuple4ub(255, 255, 255, 255);
	mAnimColors[2] = tuple4ub(255, 255, 255, 255);
	mAnimColors[3] = tuple4ub(255, 255, 255, 255);
	mAnimColors[4] = tuple4ub(255, 255, 255, 0);
	mAnimColorsNum = MAX_ANIM_COLORS;

	mSizeGrow = 0.4f;

	mForce = vec3(0,0,0);
	mRandomForce = vec3(0,0,0);

	mDumping = 1;


	SQREFL_SET_CLASS(World::ParticleSystem);

	wrapAtomicField("Emit",				&mEmit);
	wrapAtomicField("SizeRange",		&mSizeRange.x, 2);
	wrapAtomicField("EnergyRange",		&mEnergyRange.x, 2);
	wrapAtomicField("EmissionRange",	&mEmissionRange.x, 2);

	wrapAtomicField("StartVelocity",	&mStartVelocity.x, 3);
	wrapAtomicField("StartRndVelocity",	&mStartRndVelocity.x, 3);

	wrapAtomicField("Rotate",			&mRotation);
	wrapAtomicField("StartRotation",	&mStartRotation);
	wrapAtomicField("StartRndRotation",	&mStartRndRotation);

	wrapAtomicField("EmissionElipsoid",	&mEmissionElipsoid.x, 3);

	wrapAtomicField("AnimateColors",	&mAnimateColors);
	//wrapAtomicField("AnimColorsNum",	&mAnimColorsNum);
	wrapAtomicField("AnimColor1",		&mAnimColors[0].x, 4);
	wrapAtomicField("AnimColor2",		&mAnimColors[1].x, 4);
	wrapAtomicField("AnimColor3",		&mAnimColors[2].x, 4);
	wrapAtomicField("AnimColor4",		&mAnimColors[3].x, 4);
	wrapAtomicField("AnimColor5",		&mAnimColors[4].x, 4);

	wrapAtomicField("SizeGrow",			&mSizeGrow);

	wrapAtomicField("Force",			&mForce.x, 3);
	wrapAtomicField("RandomForce",		&mRandomForce.x, 3);

	wrapAtomicField("Dumping",			&mDumping);

	Reflection::Object::Field * field = wrapAtomicField<int>("RenderWay",	reinterpret_cast<int*>(&mRenderWay));
	field->attributes.push_back("Read-only");

	field = wrapAtomicField("MaxParticlesNum",	&mMaxParticlesNum);
	field->attributes.push_back("Read-only");

	field = wrapAtomicField("TextureName",		&mTextureName);
	field->setChangeHandler(this, &ParticleSystem::onTextureNameChaned);
	//field->attributes.push_back("Resource::Texture");
}

ParticleSystem::~ParticleSystem()
{
	DELETE_PTR(mMesh);
	if(mTexture)
		Resource::TextureStorage::Active()->release(mTexture->getID());
	if(mProgram)
		Resource::ProgramStorage::Active()->release(mProgram->getID());
}

void ParticleSystem::deserialize(Reflection::Deserializer * deserializer)
{
	SceneObject::deserialize(deserializer);

	init(mTextureName, mMaxParticlesNum, mRenderWay);
}

void ParticleSystem::setEmit(bool e) { 
	if(!mEmit)
	{
		mTimeLeftFromLastUpdate = 1.0f / mEmissionRange.x;
	}
	mEmit = e; 
}

void ParticleSystem::init(const std::string& textureName, int maxParticlesNum, RenderWay renderWay)
{
	//internal state members

	mTimeLeftFromLastUpdate = 0;

	//init buffer

	mMaxParticlesNum = maxParticlesNum;

	//render members

	mRenderWay = renderWay;

	mMesh = new Resource::Mesh();

	const int vertsPerParticle = 4;
	const int indsPerParticle = 6;
	int i			= 0, 
		vertsNum	= maxParticlesNum * vertsPerParticle,
		indsNum		= maxParticlesNum * indsPerParticle;

	int vertType = mRenderWay == rwSoftwareBillboards ? VT_SOFTWARE_BILLBOARDS : VT_BILLBOARDS;
	
	//init vertex buffer
	VertexBuffer * vb = mMesh->createVertexBuffer(vertType, vertsNum);
	vb->setStorageType(IBuffer::stGPUDynamicMemory);

	for(i = 0; i < vertsNum; i += vertsPerParticle)
	{
		vb->setComponent<VertexBuffer::vcTexcoord>(i + 0, vec2(1, 1));
		vb->setComponent<VertexBuffer::vcTexcoord>(i + 1, vec2(0, 1));
		vb->setComponent<VertexBuffer::vcTexcoord>(i + 2, vec2(1, 0));
		vb->setComponent<VertexBuffer::vcTexcoord>(i + 3, vec2(0, 0));
	}
	
	//init index buffer
	IndexBuffer * ib = mMesh->createIndexBuffer(indsNum);
	ib->setStorageType(IBuffer::stGPUStaticMemory);
	ib->setPolyOri(IndexBuffer::poCounterClockWise);
	ib->setPolyType(IndexBuffer::ptTriangles);

	int j = 0;
	for(i = 0; i < indsNum; i += indsPerParticle)
	{
		ib->setIndex(i + 0, j + 0);
		ib->setIndex(i + 1, j + 1);
		ib->setIndex(i + 2, j + 2);
		ib->setIndex(i + 3, j + 2);
		ib->setIndex(i + 4, j + 1);
		ib->setIndex(i + 5, j + 3);
		j += 4;
	}

	//obtain texture
	mTexture = Resource::TextureStorage::Active()->loadTexture(textureName);
	mTextureName = textureName;

	//obtain program
	mProgram = Resource::ProgramStorage::Active()->add("billboards.glsl");
}

void ParticleSystem::onTextureNameChaned()
{
	if(mTexture != NULL)
	{
		Resource::TextureStorage::Active()->release( mTexture->getID() );
	}
	mTexture = Resource::TextureStorage::Active()->add(mTextureName);
}

void ParticleSystem::update(float dtime)
{
	if(dtime > 1.0f)
		dtime = 1.0f;

	float scale = getScale().x;

	mAABB.reset();

	//update particles

	std::list<Particle>::iterator it = mParticles.begin();
	while(it != mParticles.end())
	{
		Particle& particle = (*it);

		particle.velocity.x += (mForce.x + getRandomMinMax(-mRandomForce.x, mRandomForce.x)) * dtime;
		particle.velocity.y += (mForce.y + getRandomMinMax(-mRandomForce.y, mRandomForce.y)) * dtime;
		particle.velocity.z += (mForce.z + getRandomMinMax(-mRandomForce.z, mRandomForce.z)) * dtime;

		particle.pos += particle.velocity * scale * dtime;
		
		particle.energy -= dtime;//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		if(particle.energy <= 0)
		{
			it = mParticles.erase(it);
			continue;
		}
		
		float lifeElapsed = (particle.initialEnergy - particle.energy) / particle.initialEnergy;

		particle.size += mSizeGrow * scale * dtime;

		float colorFactor = lifeElapsed * mAnimColorsNum;

		int colorIndex = (int)floor(colorFactor);
		int nextColorIndex = colorIndex + 1;
		float colorFrac = colorFactor - colorIndex;

		if((nextColorIndex) < mAnimColorsNum)
			particle.color = lerp(	colorBytesToVec4(mAnimColors[colorIndex]), 
									colorBytesToVec4(mAnimColors[nextColorIndex]), colorFrac);
		else
			particle.color = colorBytesToVec4(mAnimColors[mAnimColorsNum - 1]);
			
		if(mRotation)
		{
			particle.angle += particle.angularVelocity * dtime;
		}

		mAABB.addVertex(particle.pos);

		++it;
	}

	//emit particles

	float deltaTimesSum = dtime + mTimeLeftFromLastUpdate;

	float emitsPerSecond = mEmissionRange.x;

	float timePerEmit = 1.0f / emitsPerSecond;

	float emitsNumFloat = deltaTimesSum / timePerEmit;

	int emitsNum = (int)floor(emitsNumFloat);

	mTimeLeftFromLastUpdate = deltaTimesSum - emitsNum * timePerEmit;

	if(mEmit)
	{
		for(int i = 0; i < emitsNum; ++i)
		{
			emitParticle();
		}
	}
}

void ParticleSystem::renderCustom(Render::IRender * render, Render::Camera * camera, const RenderInfo& info)
{
	if(camera != NULL)
	{
		if(!isInCamera(camera))
		{
			return;
		}
	}

	if(mMesh == NULL)
	{
		init("", MAX_PARTICLES_NUM, rwBillboards);
	}

	VertexBuffer * vb = mMesh->getVertexBuffer();

	int vertex = 0;
	std::list<Particle>::iterator it;
	
	switch(mRenderWay)
	{
	case rwSoftwareBillboards:
		{
			vec3 eye = camera->getPosition() - getPosition();
			vec3 right = (eye ^ vec3(0, 1, 0)).normalized();
			vec3 up = (right ^ eye).normalized();

			for(it = mParticles.begin(); it != mParticles.end(); ++it)
			{
				//TODO: add support of rotation if needed
				//TODO: calc right and up vectors for each particle separatelly
				Particle& particle = (*it);

				vec3 particleUp		= up * particle.size * 0.5;
				vec3 particleRight	= right * particle.size * 0.5;

				vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
				vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos + particleUp + particleRight);
				vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
				vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos - particleUp + particleRight);
				vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
				vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos + particleUp - particleRight);
				vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
				vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos - particleUp - particleRight);
			}
		}
		break;
	case rwBillboards:

		for(it = mParticles.begin(); it != mParticles.end(); ++it)
		{
			Particle& particle = (*it);
			float angle = particle.angle * DEG2RAD;
			vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
			vb->setComponent<VertexBuffer::vcTexcoord2>(vertex, vec2(particle.size, angle));
			vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos);
			vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
			vb->setComponent<VertexBuffer::vcTexcoord2>(vertex, vec2(particle.size, angle));
			vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos);
			vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
			vb->setComponent<VertexBuffer::vcTexcoord2>(vertex, vec2(particle.size, angle));
			vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos);
			vb->setComponent<VertexBuffer::vcColor>(	vertex, particle.color);
			vb->setComponent<VertexBuffer::vcTexcoord2>(vertex, vec2(particle.size, angle));
			vb->setComponent<VertexBuffer::vcPosition>(	vertex++, particle.pos);
		}
		break;
	case rwSoftBillboards:
	case rwPointSprites:
	default:
		ASSERT(false);//not implemented
	}
	
	vb->update(0, mParticles.size() * 4 * vb->getVertexSize());

	if(mTexture != NULL)
	{
		mTexture->getRenderTexture()->bind();
	}

	Render::IProgram * program = mProgram->getRenderProgram(mRotation ? "ROTATION;" : "");
	program->bind();
	program->uniform("particleMap", 0);
	program->uniform("cameraPos", camera->getPosition());
	program->uniform("cameraUp", vec3(0,1,0));

	render->setBlendMode(Render::IRender::blendSprites);
	render->setAlphaTestValue(0);
	render->enableDepthWrite(false);
	render->setColor(vec4(1,1,1,1));
	//render->enableDepthTest(false);

	render->setTransform(mat4::Identity());
	render->setupVertexBuffer(vb);

	render->getUniformsPool().fetchUniforms(program);

	render->renderIndexBuffer(mMesh->getIndexBuffer(), tuple2i(0, mParticles.size() * 6));

	render->enableDepthWrite(true);
	//render->enableDepthTest(true);
}

void ParticleSystem::emitParticle()
{
	if(mParticles.size() >= mMaxParticlesNum) return;

	float scale = getScale().x;

	mParticles.push_back(Particle());
	Particle& particle = mParticles.back();

	particle.pos.x = getRandomMinMax(-mEmissionElipsoid.x, mEmissionElipsoid.x) * scale;
	particle.pos.y = getRandomMinMax(-mEmissionElipsoid.y, mEmissionElipsoid.y) * scale;
	particle.pos.z = getRandomMinMax(-mEmissionElipsoid.z, mEmissionElipsoid.z) * scale;

	particle.pos += getPosition();

	particle.angle = getRandomMinMax(0, mStartRndRotation);

	particle.color = colorBytesToVec4(mAnimColors[0]);

	particle.energy = getRandomMinMax(mEnergyRange.x, mEnergyRange.y);
	particle.initialEnergy = particle.energy;
	particle.size = getRandomMinMax(mSizeRange.x, mSizeRange.y) * scale;

	particle.velocity = mStartVelocity;
	particle.velocity.x += getRandomMinMax(0, mStartRndVelocity.x);
	particle.velocity.y += getRandomMinMax(0, mStartRndVelocity.y);
	particle.velocity.z += getRandomMinMax(0, mStartRndVelocity.z);

	if(mRotation)
		particle.angularVelocity = mStartRotation + getRandomMinMax(0, mStartRndRotation);
}

void ParticleSystem::calcAABB()
{
	float scale = getScale().x;
	mAABB.grow(mSizeRange.y);
	mAABB.scale(vec3(scale, scale, scale), mAABB.getCenter());
}

}//namespace World { 
}//namespace Squirrel {
