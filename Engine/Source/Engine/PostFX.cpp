#include "PostFX.h"
#include <Render/IRender.h>
#include <Render/Camera.h>
#include <Resource/Mesh.h>
#include <Math/BasicUtils.h>
#include <pugixml/pugixml.hpp>

namespace Squirrel {
namespace Engine {

#define FX_TAG					"PostFX"
#define RENDER_TERGET_TAG		"RenderTarget"
#define COLOR_TAG				"Color"
#define SHADER_TAG				"Shader"
#define RESOURCE_TAG			"Resource"
#define PARAMETERS_TAG			"Parameters"
#define INPUT_TEXTURE_TAG		"InputTexture"
#define TARGET_OUTPUT_TAG		"TargetOutput"
#define SCENE_BUFFER_TAG		"SceneBuffer"
#define INPUT_VALUE_TAG			"InputValue"
#define PASS_TAG				"Pass"
#define SEQUENCE_TAG			"Sequence"
#define RUN_PASS_TAG			"RunPass"

#define NAME_ATTR				"name"
#define WIDTH_ATTR				"width"
#define HEIGHT_ATTR				"height"
#define INDEX_ATTR				"index"
#define FORMAT_ATTR				"format"
#define TARGET_ATTR				"target"
#define REQUEST_FORMAT_ATTR		"requestFormat"
#define TYPE_ATTR				"type"
#define ARRAY_SIZE_ATTR			"arraySize"
#define SHADER_ATTR				"shader"
#define CONTENT_ATTR			"content"

#define BACKBUFFER_NAME			"backbuffer"

ITexture::PixelFormat parseTextureFormat(const char_t * formatString)
{
	if(strcmp(formatString, "RGBA8") == 0)
		return ITexture::pfColor4Byte;
	if(strcmp(formatString, "RGB8") == 0)
		return ITexture::pfColor3Byte;
	if(strcmp(formatString, "R32F") == 0)
		return ITexture::pfColor1Float;
	if(strcmp(formatString, "R16F") == 0)
		return ITexture::pfColor1Half;
	if(strcmp(formatString, "RG32F") == 0)
		return ITexture::pfColor2Float;
	if(strcmp(formatString, "RG16F") == 0)
		return ITexture::pfColor2Half;
	if(strcmp(formatString, "RGBA32F") == 0)
		return ITexture::pfColor4Float;
	if(strcmp(formatString, "RGBA16F") == 0)
		return ITexture::pfColor4Half;
	if(strcmp(formatString, "D24") == 0)
		return ITexture::pfDepth24;
	if(strcmp(formatString, "D16") == 0)
		return ITexture::pfDepth16;
	if(strcmp(formatString, "D32") == 0)
		return ITexture::pfDepth32;

	return ITexture::pfUnknown;
}

bool PostFX::parseShaderInput(PostFX::ShaderInput& shaderInput, pugi::xml_node node)
{
	UniformString name;

	pugi::xml_attribute attr;

	for(pugi::xml_node inputTexNode = node.child(INPUT_TEXTURE_TAG);
		inputTexNode;
		inputTexNode = inputTexNode.next_sibling(INPUT_TEXTURE_TAG))
	{
		std::shared_ptr<PostFX::ShaderInput::TextureInput> textureInput(new PostFX::ShaderInput::TextureInput);
		shaderInput.textureInputs.push_back(textureInput);

		attr = inputTexNode.attribute(NAME_ATTR);
		if(attr)
			name = attr.value();

		textureInput->uniformName = name;

		bool typeIsDetermined = false;

		pugi::xml_node targetOutputNode = inputTexNode.child(TARGET_OUTPUT_TAG);
		if(targetOutputNode)
		{
			textureInput->type = PostFX::ShaderInput::TextureInput::targetOutput;

			attr = targetOutputNode.attribute(TARGET_ATTR);
			if(attr)
				textureInput->targetName = attr.value();

			attr = targetOutputNode.attribute(INDEX_ATTR);
			if(attr)
				textureInput->targetAttachment = atoi(attr.value());

			typeIsDetermined = true;
		}

		pugi::xml_node resourceNode = inputTexNode.child(RESOURCE_TAG);
		if(resourceNode)
		{
			textureInput->type = PostFX::ShaderInput::TextureInput::textureResource;

			textureInput->texName = resourceNode.child_value();

			textureInput->texResource = Resource::TextureStorage::Active()->add( textureInput->texName );
			ASSERT(textureInput->texResource);

			typeIsDetermined = true;
		}

		for(pugi::xml_node bufferNode = inputTexNode.child(SCENE_BUFFER_TAG);
			bufferNode;
			bufferNode = bufferNode.next_sibling(SCENE_BUFFER_TAG))
		{
			textureInput->type = PostFX::ShaderInput::TextureInput::sceneBuffer;

			attr = bufferNode.attribute(CONTENT_ATTR);
			if(attr)
				textureInput->bufferName = attr.value();

			attr = bufferNode.attribute(REQUEST_FORMAT_ATTR);
			if(attr)
				textureInput->requestFormat = parseTextureFormat(attr.value());
			else
				textureInput->requestFormat = ITexture::pfUnknown;

			mBuffersProvider->prepareBuffer(textureInput->bufferName, textureInput->requestFormat);

			typeIsDetermined = true;
		}
		
		ASSERT(typeIsDetermined);
	}

	for(pugi::xml_node valueNode = node.child(INPUT_VALUE_TAG);
		valueNode;
		valueNode = valueNode.next_sibling(INPUT_VALUE_TAG))
	{
		attr = valueNode.attribute(NAME_ATTR);
		if(attr)
			name = attr.value();

		std::string type;
		attr = valueNode.attribute(TYPE_ATTR);
		if(attr)
			type = attr.value();
		
		size_t arraySize = 1;
		attr = valueNode.attribute(ARRAY_SIZE_ATTR);
		if(attr)
			arraySize = atoi(attr.value());

		ASSERT(valueNode.child_value());

		std::vector<std::string> valueStrings;
		split(valueNode.child_value(), " \t\n\r", valueStrings);

		size_t valuesNum = valueStrings.size();

		std::vector<float> values(valuesNum);

		for(size_t i = 0; i < valuesNum; ++i)
			values[i] = (float)atof(valueStrings[i].c_str());

		if(type == "float")
			shaderInput.valueInputs.uniformArray(name, arraySize, values.data());
		else if(type == "vec2")
			shaderInput.valueInputs.uniformArray(name, arraySize, (vec2 *)values.data());
		else if(type == "vec3")
			shaderInput.valueInputs.uniformArray(name, arraySize, (vec3 *)values.data());
		else if(type == "vec4")
			shaderInput.valueInputs.uniformArray(name, arraySize, (vec4 *)values.data());
		else if(type == "mat3")
			shaderInput.valueInputs.uniformArray(name, arraySize, (mat3 *)values.data());
		else if(type == "mat4")
			shaderInput.valueInputs.uniformArray(name, arraySize, (mat4 *)values.data());
	}

	return true;
}

int parseScreenSizeValue(const char_t * source, int screenValue)
{
	std::string str(source);
	size_t signPos = str.find('%');
	if(signPos != std::string::npos)
	{
		double percents = atof(str.substr(0, signPos).c_str());
		return (int)(screenValue * (percents/100));
	}
	else
	{
		return atoi(source);
	}
}

PostFX::PostFX(const char_t * source, SceneBuffersProvider * buffersProvider):
	mBuffersProvider(buffersProvider)
{
	pugi::xml_document xmlDoc;

	xmlDoc.load(source);

	pugi::xml_node fxNode = xmlDoc.child(FX_TAG);

	if(!fxNode)
		return;

	IRender * render = IRender::GetActive();
	tuple2i screenSize = render->getWindow()->getSize();

	UniformString name;

	pugi::xml_attribute attr = fxNode.attribute(NAME_ATTR);
	if(attr)
		name = attr.value();

	for(pugi::xml_node targetNode = fxNode.child(RENDER_TERGET_TAG);
		targetNode;
		targetNode = targetNode.next_sibling(RENDER_TERGET_TAG))
	{
		attr = targetNode.attribute(NAME_ATTR);
		if(attr)
			name = attr.value();

		int width = 0;
		int height = 0;

		attr = targetNode.attribute(WIDTH_ATTR);
		if(attr)
			width = parseScreenSizeValue(attr.value(), screenSize.x);
		
		attr = targetNode.attribute(HEIGHT_ATTR);
		if(attr)
			height = parseScreenSizeValue(attr.value(), screenSize.y);

		std::shared_ptr<IFrameBuffer> frameBuffer(render->createFrameBuffer(width, height, 0));
		mRenderTargets[name] = frameBuffer;

		frameBuffer->generate();
		frameBuffer->create();
		frameBuffer->bind();

		bool colorAttachment = false;

		for(pugi::xml_node colorAttachmentNode = targetNode.child(COLOR_TAG);
			colorAttachmentNode;
			colorAttachmentNode = colorAttachmentNode.next_sibling(COLOR_TAG))
		{
			int index = 0;

			attr = colorAttachmentNode.attribute(INDEX_ATTR);
			if(attr)
				index = atoi(attr.value());

			ITexture::PixelFormat textureFormat = ITexture::pfColor4Byte;
			
			attr = colorAttachmentNode.attribute(FORMAT_ATTR);
			if(attr)
				textureFormat = parseTextureFormat(attr.value());

			ITexture * colorTexture = render->createTexture();
			ASSERT(colorTexture);

			colorAttachment = colorTexture->fill(textureFormat, tuple3i(width, height, 1));
			ASSERT(colorAttachment);

			colorAttachment = frameBuffer->attachColorTexture(colorTexture, index);
			ASSERT(colorAttachment);
		}

		if(colorAttachment)
		{
			const float lum = 1.0f;
			render->setColor(vec4(lum, lum, lum, lum));
			render->clear(true, false);
		}

		frameBuffer->unbind();
	}

	for(pugi::xml_node shaderNode = fxNode.child(SHADER_TAG);
		shaderNode;
		shaderNode = shaderNode.next_sibling(SHADER_TAG))
	{
		attr = shaderNode.attribute(NAME_ATTR);
		if(attr)
			name = attr.value();

		std::shared_ptr<Shader> shader(new Shader());
		mShaders[name] = shader;

		pugi::xml_node resourceNode = shaderNode.child(RESOURCE_TAG);
		if(resourceNode)
			shader->program = Resource::ProgramStorage::Active()->add( resourceNode.child_value() );

		ASSERT(shader->program);

		pugi::xml_node paramsNode = shaderNode.child(PARAMETERS_TAG);
		if(paramsNode)
			shader->params = paramsNode.child_value();

		bool localResult = parseShaderInput(shader->input, shaderNode);
		ASSERT(localResult);
	}

	for(pugi::xml_node passNode = fxNode.child(PASS_TAG);
		passNode;
		passNode = passNode.next_sibling(PASS_TAG))
	{
		attr = passNode.attribute(NAME_ATTR);
		if(attr)
			name = attr.value();

		std::shared_ptr<Pass> pass(new Pass());
		mPasses[name] = pass;

		attr = passNode.attribute(TARGET_ATTR);
		if(attr)
			pass->targetName = attr.value();
		
		attr = passNode.attribute(SHADER_ATTR);
		if(attr)
			pass->shaderName = attr.value();
		
		bool localResult = parseShaderInput(pass->shaderInput, passNode);
		ASSERT(localResult);
	}

	for(pugi::xml_node sequenceNode = fxNode.child(SEQUENCE_TAG);
		sequenceNode;
		sequenceNode = sequenceNode.next_sibling(SEQUENCE_TAG))
	{
		attr = sequenceNode.attribute(NAME_ATTR);
		if(attr)
			name = attr.value();

		std::shared_ptr<Sequence> sequence(new Sequence());
		mSequences.push_back(sequence);

		for(pugi::xml_node runPassNode = sequenceNode.child(RUN_PASS_TAG);
			runPassNode;
			runPassNode = runPassNode.next_sibling(RUN_PASS_TAG))
		{
			attr = runPassNode.attribute(NAME_ATTR);
			if(attr)
				name = attr.value();

			sequence->runList.push_back(name);
		}
	}

	mLastSequence = mSequences.end();
}

PostFX::~PostFX()
{
}

void PostFX::drawFullScreenQuad(float w, float h)
{
	IRender * render = IRender::GetActive();

	render->enableDepthTest(Render::IRender::depthTestOff);

	static Resource::Mesh * mesh = NULL;
	if(!mesh)
	{
		mesh = new Resource::Mesh;

		VertexBuffer * vb = mesh->createVertexBuffer(VT_PT, 4);
		vb->setStorageType(IBuffer::stCPUMemory);

		IndexBuffer * ib = mesh->createIndexBuffer(6);
		ib->setPolyOri(IndexBuffer::poCounterClockWise);
		ib->setStorageType(IBuffer::stGPUStaticMemory);

		ib->setIndex(0, 0);
		ib->setIndex(1, 1);
		ib->setIndex(2, 3);
		ib->setIndex(3, 3);
		ib->setIndex(4, 1);
		ib->setIndex(5, 2);
	}

	VertexBuffer * vb = mesh->getVertexBuffer();

	float texW = 1.0f;
	float texH = 1.0f;

	vb->setComponent<VertexBuffer::vcPosition>(0, vec3(0, 0, 0));
	vb->setComponent<VertexBuffer::vcPosition>(1, vec3(w, 0, 0));
	vb->setComponent<VertexBuffer::vcPosition>(2, vec3(w, h, 0));
	vb->setComponent<VertexBuffer::vcPosition>(3, vec3(0, h, 0));

	vb->setComponent<VertexBuffer::vcTexcoord>(3, vec2(0.0f, 0.0f));
	vb->setComponent<VertexBuffer::vcTexcoord>(2, vec2(texW, 0.0f));
	vb->setComponent<VertexBuffer::vcTexcoord>(1, vec2(texW, texH));
	vb->setComponent<VertexBuffer::vcTexcoord>(0, vec2(0.0f, texH));

	render->setupVertexBuffer(vb);
	render->renderIndexBuffer(mesh->getIndexBuffer());
}

bool PostFX::setupShaderInput(ShaderInput& input, IProgram * program)
{
	FOREACH(ShaderInput::TEXTURE_INPUTS_LIST::iterator, itTex, input.textureInputs)
	{
		ShaderInput::TextureInput * texInput = itTex->get();

		int unit = program->getSamplerUnit(texInput->uniformName);

		if(unit < 0)
			continue;

		ITexture * texture = NULL;

		switch(texInput->type)
		{
			case ShaderInput::TextureInput::targetOutput:
			{
				RENDER_TARGETS_MAP::iterator itTarget = mRenderTargets.find(texInput->targetName);

				if(itTarget == mRenderTargets.end())
				{
					ASSERT(false);
					continue;
				}

				IFrameBuffer * renderTarget = itTarget->second.get();

				texture = renderTarget->getAttachement(texInput->targetAttachment);
				break;
			}
			case ShaderInput::TextureInput::textureResource:
				texture = texInput->texResource->getRenderTexture();
				break;
			case ShaderInput::TextureInput::sceneBuffer:
				texture = mBuffersProvider->getBuffer(texInput->bufferName);
				break;
		}

		ASSERT(texture);

		if(texture)
		{
			texture->bind(unit);
		}
	}

	input.valueInputs.fetchUniforms(program);

	return true;
}

bool PostFX::run(Render::IFrameBuffer * finalRenderBuffer)
{
	if(mSequences.size() == 0)
		return false;

	if(mLastSequence == mSequences.end())
		mLastSequence = mSequences.begin();
	else
	{
		++mLastSequence;
		if(mLastSequence == mSequences.end())
			mLastSequence = mSequences.begin();
	}

	Sequence * sequence = mLastSequence->get();

	FOREACH(Sequence::PASSES_LIST::iterator, itPassName, sequence->runList)
	{
		//obtain pass
		
		PASSES_MAP::iterator itPass = mPasses.find(*itPassName);

		if(itPass == mPasses.end())
		{
			ASSERT(false);
			continue;
		}

		Pass * pass = itPass->second.get();

		//obtain renderTarget
		
		RENDER_TARGETS_MAP::iterator itTarget = mRenderTargets.find(pass->targetName);

		IFrameBuffer * renderTarget = NULL;

		if(itTarget == mRenderTargets.end())
		{
			if(pass->targetName == BACKBUFFER_NAME)
			{
				renderTarget = finalRenderBuffer;
			}
			else
			{
				ASSERT(false);
				continue;
			}
		}
		else
			renderTarget = itTarget->second.get();

		//obtain shader

		SHADERS_MAP::iterator itShader = mShaders.find(pass->shaderName);

		if(itShader == mShaders.end())
		{
			ASSERT(false);
			continue;
		}

		Shader * shader = itShader->second.get();

		//setup and render pass

		IProgram * program = shader->program->getRenderProgram(shader->params);
		ASSERT(program);

		tuple2i screenSize = IRender::GetActive()->getWindow()->getSize();

		if(renderTarget)
		{
			renderTarget->bind();
		}
		else
		{
			IFrameBuffer::Unbind();
			IRender::GetActive()->setViewport(0, 0, screenSize.x, screenSize.y);
		}

		program->bind();

		setupShaderInput(shader->input, program);
		setupShaderInput(pass->shaderInput, program);
		mBuffersProvider->fetchPostFXUniforms(program);

		IRender * render = IRender::GetActive();

		render->setTransform(mat4::Identity());
		render->getUniformsPool().fetchUniforms(program);

		//use screen size as size for quad, as projection matrix has the same view ortho size
		//(view ortho size could be different from viewport size!!!)
		drawFullScreenQuad(screenSize.x, screenSize.y);
	}

	return true;
}

}//namespace Engine { 
}//namespace Squirrel {
