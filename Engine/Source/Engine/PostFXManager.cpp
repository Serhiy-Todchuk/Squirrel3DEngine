#include "PostFXManager.h"
#include <FileSystem/Path.h>

namespace Squirrel {
namespace Engine {

#define POST_FX_SETTINGS_SECTION "PostFX"

UniformString sSceneColorBufferName		("sceneColor");
UniformString sColorBufferName			("color");
UniformString sDepthBufferName			("depth");
UniformString sNormalsBufferName		("normal");
UniformString sSpeedBufferName			("speed");

PostFXManager::PostFXManager()
{
}
	
PostFXManager::~PostFXManager()
{
}
	
void PostFXManager::init(Settings * settings)
{
	//init storage

	std::string path = settings->getString(POST_FX_SETTINGS_SECTION, "StoragePath", "PostFX");
	path = FileSystem::Path::GetAbsPath(path);

	mStorage.reset( FileSystem::FileStorageFactory::GetFileStorageForPath(path) );
	ASSERT(mStorage.get());

	//add effects
	
	std::string effectsStr = settings->getString(POST_FX_SETTINGS_SECTION, "Effects", "");

	std::list<std::string> effectsList;
	split(effectsStr, ";", effectsList);

	FOREACH(std::list<std::string>::const_iterator, itFX, effectsList)
	{
		const std::string& fxFileName = *itFX;
		add(fxFileName);
	}
}
	
PostFX * PostFXManager::add(const UniformString& fxFilePath)
{
	if(fxFilePath.length() == 0)
		return NULL;

	Data * data = mStorage->getFile(fxFilePath.c_str());
	if(!data)
		return NULL;

	PostFX * fx = new PostFX(static_cast<const char_t *>(data->getData()), this);
	
	mFXList.push_back(std::shared_ptr<PostFX>(fx));

	return fx;
}
	
void PostFXManager::process()
{
	int counter = 0;

	FOREACH(POSTFX_LIST::iterator, it, mFXList)
	{
		IFrameBuffer * target = NULL;

		if(counter % 2 == 0)
		{
			target = mColorEvenFrameBuffer.get();
			mScreenBuffers[sColorBufferName] =	counter == 0 ?
												mSceneFrameBuffer->getAttachement() :
												mColorOddFrameBuffer->getAttachement();
		}
		else
		{
			target = mColorOddFrameBuffer.get();
			mScreenBuffers[sColorBufferName] = mColorEvenFrameBuffer->getAttachement();
		}

		POSTFX_LIST::iterator itNext = it;
		if(++itNext == mFXList.end())//if last fx
			target = NULL;

		(*it)->run(target);

		++counter;
	}
}

//implement SceneBuffersProvider
ITexture * PostFXManager::getBuffer(const UniformString& buffName)
{
	SCREEN_BUFFERS_MAP::iterator it = mScreenBuffers.find(buffName);
	if(it != mScreenBuffers.end())
		return it->second;
	return NULL;
}
	
bool PostFXManager::prepareBuffer(const UniformString& buffName, ITexture::PixelFormat format)
{
	if(buffName != sColorBufferName &&
	   buffName != sDepthBufferName &&
	   buffName != sSceneColorBufferName)
		return false;
	
	IRender * render = IRender::GetActive();
	tuple2i screenSize = render->getWindow()->getSize();

	if(format == ITexture::pfUnknown)
		format = ITexture::pfColor4Byte;

	if(mSceneFrameBuffer.get() == NULL)
	{
		mSceneFrameBuffer.reset(render->createFrameBuffer(screenSize.x, screenSize.y, 0));
		mSceneFrameBuffer->generate();
		mSceneFrameBuffer->create();
		mSceneFrameBuffer->bind();
		
		//create color buffer
		ITexture * colorBuffer = render->createTexture();
		colorBuffer->generate();
		colorBuffer->fill(format, tuple3i(screenSize.x, screenSize.y, 1));
		mSceneFrameBuffer->attachColorTexture(colorBuffer);
		mScreenBuffers[sColorBufferName] = colorBuffer;
		mScreenBuffers[sSceneColorBufferName] = colorBuffer;

		//create depth buffer
		ITexture * depthBuffer = render->createTexture();
		depthBuffer->generate();
		depthBuffer->fill(ITexture::pfDepth24, tuple3i(screenSize.x, screenSize.y, 1));
		mSceneFrameBuffer->attachDepthTexture(depthBuffer);
		mScreenBuffers[sDepthBufferName] = depthBuffer;
		
		mSceneFrameBuffer->isOk();
	}
	if(mColorOddFrameBuffer.get() == NULL)
	{
		mColorOddFrameBuffer.reset(render->createFrameBuffer(screenSize.x, screenSize.y, 0));
		mColorOddFrameBuffer->generate();
		mColorOddFrameBuffer->create();
		mColorOddFrameBuffer->bind();

		//create color buffer
		ITexture * colorBuffer = render->createTexture();
		colorBuffer->generate();
		colorBuffer->fill(format, tuple3i(screenSize.x, screenSize.y, 1));
		mColorOddFrameBuffer->attachColorTexture(colorBuffer);
		mColorOddFrameBuffer->isOk();
	}
	if(mColorEvenFrameBuffer.get() == NULL)
	{
		mColorEvenFrameBuffer.reset(render->createFrameBuffer(screenSize.x, screenSize.y, 0));
		mColorEvenFrameBuffer->generate();
		mColorEvenFrameBuffer->create();
		mColorEvenFrameBuffer->bind();

		//create color buffer
		ITexture * colorBuffer = render->createTexture();
		colorBuffer->generate();
		colorBuffer->fill(format, tuple3i(screenSize.x, screenSize.y, 1));
		mColorEvenFrameBuffer->attachColorTexture(colorBuffer);
		mColorEvenFrameBuffer->isOk();
	}

	SCREEN_BUFFERS_MAP::iterator it = mScreenBuffers.find(buffName);
	if(it == mScreenBuffers.end())
		mScreenBuffers[buffName] = NULL;
	
	return true;
}

}//namespace Engine { 
}//namespace Squirrel {
