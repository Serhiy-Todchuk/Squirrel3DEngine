#pragma once

#include <Render/Uniform.h>
#include <Render/IRender.h>
#include <Render/ITexture.h>
#include <Render/IFrameBuffer.h>
#include <Resource/TextureStorage.h>
#include <Resource/ProgramStorage.h>
#include "macros.h"

namespace pugi {
	class xml_node;
}

namespace Squirrel {
namespace Engine {

using namespace Render;

class SQENGINE_API SceneBuffersProvider
{
protected:
	UniformContainer mPostFXUniforms;

public:
	virtual ~SceneBuffersProvider() {}

	void fetchPostFXUniforms(IProgram * program) {
		mPostFXUniforms.fetchUniforms(program);
	}

	virtual ITexture * getBuffer(const UniformString& buffName) = 0;
	virtual bool prepareBuffer(const UniformString& buffName, ITexture::PixelFormat format = ITexture::pfUnknown) = 0;
};

class SQENGINE_API PostFX
{
public:

	struct ShaderInput
	{
		struct TextureInput
		{
			TextureInput(): texResource(NULL) {}
			~TextureInput() {
				if(texResource)
					Resource::TextureStorage::Active()->release(texResource->getID());
			}

			enum Type {
				targetOutput,
				textureResource,
				sceneBuffer
			};

			UniformString uniformName;

			Type type;

			UniformString targetName;//for targetOutput type
			int targetAttachment;//for targetOutput type, index of color attachment

			ITexture::PixelFormat requestFormat;//for sceneBuffer type
			std::string bufferName;//for sceneBuffer type

			std::string texName;//for textureResource type
			
			Resource::Texture * texResource;//for textureResource type
		};

		typedef std::list< std::shared_ptr<TextureInput> > TEXTURE_INPUTS_LIST;

		UniformContainer	valueInputs;
		TEXTURE_INPUTS_LIST	textureInputs;
	};

private:

	struct Shader
	{
		Shader(): program(NULL) {}
		~Shader() {
			if(program)
				Resource::ProgramStorage::Active()->release(program->getID());
		}

		Resource::Program * program;

		std::string params;

		ShaderInput input;
	};

	struct Pass
	{
		UniformString targetName;

		UniformString shaderName;
		
		ShaderInput shaderInput;
	};

	struct Sequence
	{
		typedef std::list<UniformString> PASSES_LIST;

		UniformString name;

		PASSES_LIST runList;
	};
	
	typedef UniformMap<UniformString, std::shared_ptr<IFrameBuffer> >	RENDER_TARGETS_MAP;
	typedef UniformMap<UniformString, std::shared_ptr<Shader> >			SHADERS_MAP;
	typedef UniformMap<UniformString, std::shared_ptr<Pass> >			PASSES_MAP;
	typedef std::list< std::shared_ptr<Sequence> >						SEQUENCES_LIST;

	RENDER_TARGETS_MAP	mRenderTargets;
	SHADERS_MAP			mShaders;
	PASSES_MAP			mPasses;
	SEQUENCES_LIST		mSequences;

	UniformString		mName;

	SceneBuffersProvider * mBuffersProvider;

	SEQUENCES_LIST::iterator	mLastSequence;

private:

	bool setupShaderInput(ShaderInput& input, IProgram * program);

	void drawFullScreenQuad(float w, float h);

	bool parseShaderInput(PostFX::ShaderInput& shaderInput, pugi::xml_node node);

public:

	PostFX(const char_t * source, SceneBuffersProvider * buffersProvider);
	~PostFX();

	const UniformString& getName() const { return mName; }
	void setName(const UniformString& name) { mName = name; }

	bool run(Render::IFrameBuffer * finalRenderBuffer = NULL);
};


}//namespace Engine { 
}//namespace Squirrel {