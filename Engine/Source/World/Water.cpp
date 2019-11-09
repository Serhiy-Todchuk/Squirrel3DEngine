#include "Water.h"
#include <iomanip>

namespace Squirrel {
namespace World {

Render::UniformString sUniformNormalMap	("normalMap");

Water::Water(): mMesh(NULL)
{
}

Water::~Water() 
{
	DELETE_PTR(mMesh);
	for(size_t i = 0; i < mTextures.size(); ++i)
		Resource::TextureStorage::Active()->release(mTextures[i]->getID());
}

void Water::init(const std::string& texFileName)
{
	using namespace RenderData;
	
	Resource::PlaneBuilder * meshBuilder = new Resource::PlaneBuilder( 1000, 1000, 8, 8, VT_PNT );

	mMesh = meshBuilder->buildMesh();

	DELETE_PTR(meshBuilder);

	mMesh->getVertexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mMesh->getIndexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mMesh->getIndexBuffer()->setPolyOri(IndexBuffer::poCounterClockWise);

	mMesh->calcBoundingVolume();

	size_t maskPos = texFileName.find_first_of('*');
	if(maskPos == std::string::npos)
	{
		mTextures.push_back( Resource::TextureStorage::Active()->add(texFileName) );
	}
	else
	{
		size_t maskEnd = texFileName.find_last_of('*');
		size_t maskLength = maskEnd - maskPos + 1;
		size_t maxFramesNum = (size_t)pow(10.0, (double)maskLength);
		for(size_t i = 0; i < maxFramesNum; ++i)
		{
			std::stringstream fileNameStr;

			fileNameStr << texFileName.substr(0, maskPos);
			fileNameStr.fill('0');
			fileNameStr << std::setw((int)maskLength) << std::right << i;
			fileNameStr << std::setw(0) << texFileName.substr(maskEnd + 1);

			const std::string& str = fileNameStr.str();
			Resource::Texture * tex = Resource::TextureStorage::Active()->loadTexture(str, true, true);
			if(tex)
				mTextures.push_back( tex );
		}
	}
}

void Water::render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	ASSERT(mMesh != NULL);

	Render::MaterialGroup * matGroup = renderQueue->beginMaterialGroup();

	matGroup->mRequiresReflection = true;
	matGroup->mReflectionDesc.mReflectionPos = vec3(0, 0, 0);
	matGroup->mReflectionDesc.mReflectionDir = vec3(0, 1, 0);

	matGroup->mRequiresColorBuffer	= true;
	matGroup->mRequiresDepthBuffer	= true;
	matGroup->mRenderQueue			= Render::rqTransparent;
	matGroup->mRenderOnce			= true;

	matGroup->mProgramName = "water.glsl";

	const float timeBetweenFrames = 0.04f;
	float time = TimeCounter::Instance().getTime();
	int globalFrame = (int)(time / timeBetweenFrames);
	int frame = globalFrame % mTextures.size();

	matGroup->mTextures[sUniformNormalMap] = mTextures[frame]->getRenderTexture();

	vec4 waterColor = vec4(0.5f, 0.7f, 0.9f, 1.0f);
	float waterDensity = 0.5f;

	matGroup->mUniformsPool.uniformArray("waterColor", 1, &waterColor);
	matGroup->mUniformsPool.uniformArray("waterDensity", 1, &waterDensity);
	//matGroup->mUniformsPool.uniformArray("specular", 1, &waterDensity);

	matGroup = renderQueue->endMaterialGroup();

	Render::VBGroup * vbGroup = matGroup->getVBGroup(mMesh->getVertexBuffer(), 0);

	//put index primitive

	Render::IndexPrimitive * primitive = vbGroup->getIndexPrimitive(mMesh->getIndexBuffer());

	primitive->addInstance( mat4::Identity(), mAABB );
}

void Water::calcAABB()
{
	mAABB = mMesh->getAABB();
}

}//namespace World {
}//namespace Squirrel {
