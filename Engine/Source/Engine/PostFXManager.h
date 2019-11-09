#pragma once

#include "PostFX.h"
#include <Common/Settings.h>
#include <FileSystem/FileStorage.h>

namespace Squirrel {
namespace Engine {

using namespace Render;

class SQENGINE_API PostFXManager:
	public SceneBuffersProvider
{
	typedef std::list< std::shared_ptr<PostFX> > POSTFX_LIST;

	typedef std::map<UniformString, ITexture *> SCREEN_BUFFERS_MAP;

	POSTFX_LIST mFXList;

	std::auto_ptr<FileSystem::FileStorage> mStorage;

	SCREEN_BUFFERS_MAP mScreenBuffers;

	std::auto_ptr<IFrameBuffer> mSceneFrameBuffer;
	std::auto_ptr<IFrameBuffer> mColorOddFrameBuffer;
	std::auto_ptr<IFrameBuffer> mColorEvenFrameBuffer;

public:
	PostFXManager();
	~PostFXManager();

	IFrameBuffer * getSceneFrameBuffer() { return mSceneFrameBuffer.get(); }

	UniformContainer * getPostFXUniforms() { return &mPostFXUniforms; }

	void init(Settings * settings);

	PostFX * add(const UniformString& fxFilePath);
	
	void process();

	//implement SceneBuffersProvider
	virtual ITexture * getBuffer(const UniformString& buffName);
	virtual bool prepareBuffer(const UniformString& buffName, ITexture::PixelFormat format = ITexture::pfUnknown);
};


}//namespace Engine { 
}//namespace Squirrel {