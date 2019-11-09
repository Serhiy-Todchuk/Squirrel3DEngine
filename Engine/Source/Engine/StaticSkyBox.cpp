#include "StaticSkyBox.h"

namespace Squirrel {
namespace Engine { 

StaticSkyBox::StaticSkyBox(): mProgram(NULL), mTexture(NULL), mMesh(NULL)
{
	mRotation.identity();
}

StaticSkyBox::~StaticSkyBox() 
{
	DELETE_PTR(mMesh);
	Resource::TextureStorage::Active()->release(mTexture->getID());
	Resource::ProgramStorage::Active()->release(mProgram->getID());
}

void StaticSkyBox::init(Resource::Texture * texture)
{
	init();
	mTexture = texture;

	if(mTexture == NULL)
	{
		//TODO: err msg
		return;
	}

	mTexture->getRenderTexture()->bind();
	mTexture->getRenderTexture()->setTexParameters(
		Render::ITexture::Linear, 
		Render::ITexture::ClampToEdge, 0);

}

void StaticSkyBox::init(const std::string& cubeTexture)
{
	init();

	mTexture = Resource::TextureStorage::Active()->add(cubeTexture);
	if(mTexture == NULL)
	{
		//TODO: err msg
		return;
	}

	mTexture->getRenderTexture()->bind();
	mTexture->getRenderTexture()->setTexParameters(
		Render::ITexture::Linear, 
		Render::ITexture::ClampToEdge, 0);
}

void StaticSkyBox::init()
{
	using namespace RenderData;

	const float boxSize = 100;

	Resource::SphereBuilder * meshBuilder = 
		new Resource::SphereBuilder( boxSize, 8, 8, VT_P );

	mMesh = meshBuilder->buildMesh();

	DELETE_PTR(meshBuilder);

	mMesh->getVertexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mMesh->getIndexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mMesh->getIndexBuffer()->setPolyOri(IndexBuffer::poClockWise);

	mProgram = Resource::ProgramStorage::Active()->add("sky/StaticSkyBox.glsl");
}


void StaticSkyBox::render(Render::IRender * render, Render::Camera * camera)
{
	ASSERT(mMesh != NULL);

	Math::mat4 skyTransform = mat4().identity();

	if(camera != NULL)
		skyTransform.setTranslate(camera->getPosition());

	skyTransform.setMat3(mRotation);

	if(render == NULL)
	{
		render = Render::IRender::GetActive();
		ASSERT(render != NULL);
	}

	render->setTransform(skyTransform);

	if(mTexture != NULL)
		mTexture->getRenderTexture()->bind();

	if(mProgram != NULL)
	{
		Render::IProgram * program = mProgram->getRenderProgram("");
		program->bind();
		render->getUniformsPool().fetchUniforms(program);
	}

	render->setupVertexBuffer( mMesh->getVertexBuffer() );

	render->enableDepthTest(Render::IRender::depthTestOff);
	render->enableDepthWrite(false);

	render->renderIndexBuffer( mMesh->getIndexBuffer() );

	render->enableDepthTest();
	render->enableDepthWrite(true);
}

}//namespace Engine { 
}//namespace Squirrel {
