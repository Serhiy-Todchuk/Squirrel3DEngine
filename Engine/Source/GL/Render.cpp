#include "Render.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Utils.h"
#include "Program.h"
#include "FrameBuffer.h"
#include <map>
#include <set>
#include <list>

namespace Squirrel {
namespace GL { 

using namespace RenderData;

//
// GL state management functionality
//

typedef std::set<uint> BOOL_STATES_SET;

BOOL_STATES_SET sClientStates;
BOOL_STATES_SET sStates;

bool updateBoolState(uint state, bool enable, BOOL_STATES_SET& states)
{
	BOOL_STATES_SET::iterator it = states.find(state);

	if(it != states.end())
	{
		if(!enable)
		{
			states.erase(it);
		}
		return !enable;
	}
	else
	{
		if(enable)
		{
			states.insert(state);
		}
		return enable;
	}
}
 
//
// Render static methods
//

void disableClientStates()
{
	for(BOOL_STATES_SET::iterator it = sClientStates.begin(); it != sClientStates.end(); ++it)
		glDisableVertexAttribArray((*it));
	sClientStates.clear();
}

void Render::enableClientState(uint state, bool enable)
{
	if(updateBoolState(state, enable, sClientStates))
	{
		if(enable)
		{
			//++GetActive()->getRenderStatistics().mStateSwitchesNum;
			glEnableVertexAttribArray(state);
		}
		else
		{
			//++GetActive()->getRenderStatistics().mStateSwitchesNum;
			glDisableVertexAttribArray(state);
		}
	}
}

void Render::enableState(uint state, bool enable)
{
	if(updateBoolState(state, enable, sStates))
	{
		if(enable)
		{
			//++GetActive()->getRenderStatistics().mStateSwitchesNum;
			glEnable(state);
		}
		else
		{
			//++GetActive()->getRenderStatistics().mStateSwitchesNum;
			glDisable(state);
		}
	}
}

//
// Render ctor/dtor
//

Render::Render()
{
	mLastVB = NULL;
	mCullFace = 0;
	mBlendMode = blendOff;
	mColorWrite = true;
	mDepthWrite = true;
	mAlphaTest = 0;
	mPolygonOffset = vec2(0, 0);

	enableState(GL_DEPTH_TEST,		true);
	enableState(GL_BLEND,			false);
	setAlphaTestValue(0.0f);
	
	glDepthFunc(GL_LEQUAL);
	mDepthTest = depthCompareOrEqual;
	
	glCullFace(GL_FRONT);
	
	glHint ( GL_POLYGON_SMOOTH_HINT,         GL_NICEST );
	glHint ( GL_TEXTURE_COMPRESSION_HINT,    GL_NICEST );
	
	CHECK_GL_ERROR;
	
	//if(GetActive() == NULL)
	//	setAsActive();
}

Render::~Render()
{
}

//
// Render methods
//

void Render::setColor(vec4 color)
{
	if(mColor != color)
	{
		++mStats.mStateSwitchesNum;
		mColor = color;

		mUniformsPool.uniformArray(sColorUniformName, 1, &mColor);
	}
}

void Render::setViewport(int x, int y, int width, int height)
{
	tuple4i newViewport(x, y, width, height);
	//if(newViewport == mViewport)
	{
		glViewport(x, y, width, height);
		mViewport = newViewport;
	}
}

void Render::clear(bool color, bool depth, bool stencil)
{
	uint bits = 0;
	if(color)	bits |= GL_COLOR_BUFFER_BIT;
	if(depth)	bits |= GL_DEPTH_BUFFER_BIT;
	if(stencil)	bits |= GL_STENCIL_BUFFER_BIT;
	if(bits == 0) return;
	if(color)
	{
		glClearColor(mColor.x, mColor.y, mColor.z, mColor.w);
	}
	glClear( bits );
	CHECK_GL_ERROR;
}

void Render::present()
{
	mStats.clear();

	ASSERT(mWindow);

	Context * context = mWindow->getCurrentContext();
	ASSERT(context);

	context->finishFrame();
}

void Render::flush(bool waitUntilDone)
{
	if(waitUntilDone)
		glFinish();
	else
		glFlush();
}
	
void Render::setRasterizationMode(RasterizationMode mode)
{
	GLenum glMode = GL_FILL;
	if(mode == rLine)
		glMode = GL_LINE;
	if(mode == rPoint)
		glMode = GL_POINT;

	glPolygonMode(GL_FRONT_AND_BACK, glMode);
}

void Render::enableDepthTest(DepthTestMode mode)
{
	if(mDepthTest != mode)
	{
		if(mode != depthTestOff)
		{
			GLenum glMode = mode == depthCompareOrEqual ? GL_LEQUAL : GL_EQUAL;
			glDepthFunc(glMode);
		}
		enableState(GL_DEPTH_TEST, mode != depthTestOff);
		mDepthTest = mode;
	}
	//TODO: glDepthFunc
	//TODO: stencil
}

void Render::enableDepthWrite(bool enable)
{
	if(mDepthWrite != enable)
	{
		glDepthMask(enable);
		mDepthWrite = enable;
	}
}

void Render::enableColorWrite(bool enable)
{
	if(mColorWrite != enable)
	{
		glColorMask(enable, enable, enable, enable);
		mColorWrite = enable;
	}
}

//1.0f - test always fails (object fully transparent), 
//0.0f - test always success (turned off alpha test)
void Render::setAlphaTestValue(float alpha)
{
	if(mAlphaTest != alpha)
	{
		mAlphaTest = alpha;
		mUniformsPool.uniformArray(sAlphaTestUniformName, 1, &mAlphaTest);
	}
}

void Render::setBlendMode(BlendMode mode)
{
	if(mBlendMode != mode)
	{
		enableState(GL_BLEND, mode != blendOff);
		switch(mode)
		{
		case blendAdd: 
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case blendSprites: 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case blendOneMinusAlpha: 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case blendOff:
			break;//do nothing
		};
		mBlendMode = mode;
	}
}

void Render::activeTextureUnit(int unit)
{
	Texture::SetActiveUnit(unit);
}

int Render::getActiveTextureUnit()
{
	return Texture::GetActiveUnit();
}

void Render::enablePolygonOffset(bool enable, float factor, float units)
{
	enableState(GL_POLYGON_OFFSET_FILL, enable);
	if(enable)
	{
		vec2 newPolyOffset(factor, units);
		if(mPolygonOffset != newPolyOffset)
		{
			glPolygonOffset ( factor, units );
			mPolygonOffset = newPolyOffset;
		}
	}
}

void Render::setTransform(const mat4& transform, IProgram * program)
{
	UniformReceiver * uniformReceiver = (program != NULL) ? (UniformReceiver *)program : (UniformReceiver *)&mUniformsPool;

	mModelViewMatrix	= transform;
	mMVPMatrix			= mProjectionMatrix * mModelViewMatrix;
	mat4 normalMatrix	= mModelViewMatrix;//.inverse().transposed();
	mNormalMatrix		= mat3(normalMatrix.x.getVec3(), normalMatrix.y.getVec3(), normalMatrix.z.getVec3());

	uniformReceiver->uniformArray(sModelviewMatrixUniformName, 1, &mModelViewMatrix);
	uniformReceiver->uniformArray(sMVPMatrixUniformName, 1, &mMVPMatrix);
	uniformReceiver->uniformArray(sNormalMatrixUniformName, 1, &mNormalMatrix);
}

void Render::setProjection(const mat4& projection)
{
	mProjectionMatrix = projection;
	mMVPMatrix = mProjectionMatrix * mModelViewMatrix;

	//mUniformsPool.uniformArray("uProjectionMatrix", 1, &mProjectionMatrix);
	mUniformsPool.uniformArray(sMVPMatrixUniformName, 1, &mMVPMatrix);
}

bool Render::checkError(const char_t * message)
{
	return Utils::CheckGLError(message);
}

void Render::obtainDepthBuffer(float * dstCPUBuff)
{
	glReadPixels(0, 0, getWindow()->getSize().x, getWindow()->getSize().y, GL_DEPTH_COMPONENT, GL_FLOAT, dstCPUBuff);
	CHECK_GL_ERROR;
}

void Render::setupVertexBuffer(RenderData::VertexBuffer * pVB)
{
	ASSERT(pVB);

	if(mLastVB == pVB)
		return;

	mLastVB = pVB;

	//TODO: optimize client states disabling enabling
	disableClientStates();	

	//create gl buffer if it's not created yet
	VertexBuffer * glVB = TYPE_CAST<VertexBuffer*>(pVB);
	if(!glVB->isCreated() && glVB->getStorageType() != RenderData::VertexBuffer::stCPUMemory)
	{
		glVB->create();
	}
	
	//bind gl buffer
	void * buffer = (void *)pVB->getVerts();
	if(glVB->isCreated())
	{
		buffer = 0;
		glVB->bind();
	}
	else
	{
		VertexBuffer::Unbind();
	}

	int vertexType = /*vertComponents & */pVB->getVertType();

	//texcoords

	IProgram * program = IProgram::GetBoundProgram();
	ASSERT(program != NULL);

	for(int vc = VertexBuffer::vcPosition; vc < VertexBuffer::vcNum; ++vc)
	{
#ifdef _DEBUG
		VertexBuffer::VertexComponent vertexComponent = (VertexBuffer::VertexComponent)vc;//for debugging
#endif
		if( !((VCI2VT(vc) & vertexType) || vc == VertexBuffer::vcPosition) )
			continue;

		GLenum dataType = 0;
		int attribChanel = -1;
		int normalized = GL_FALSE;
		int size = 3;

		switch(vc)
		{
		case VertexBuffer::vcPosition:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vcPosition;
			size			= 3;
			break;
		case VertexBuffer::vcNormal:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vcNormal;
			size			= 3;
			normalized		= GL_TRUE;
			break;
		case VertexBuffer::vcTangentBinormal:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vcTangentBinormal;
			size			= 4;
			break;
		case VertexBuffer::vcInt8Normal:
			dataType		= GL_BYTE;
			attribChanel	= VertexBuffer::vcNormal;
			size			= 3;
			normalized		= GL_TRUE;
			break;
		case VertexBuffer::vcInt8TangentBinormal:
			dataType		= GL_BYTE;
			attribChanel	= VertexBuffer::vcTangentBinormal;
			size			= 4;
			break;
		case VertexBuffer::vcTexcoord:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vcTexcoord;
			size			= 2;
			break;
		case VertexBuffer::vcTexcoord2:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vcTexcoord2;
			size			= 2;
			break;
		case VertexBuffer::vcInt8Texcoord:
			dataType		= GL_BYTE;
			attribChanel	= VertexBuffer::vcTexcoord;
			size			= 2;
			break;
		case VertexBuffer::vcInt8Texcoord2:
			dataType		= GL_BYTE;
			attribChanel	= VertexBuffer::vcTexcoord2;
			size			= 2;
			break;
		case VertexBuffer::vcInt16Texcoord:
			dataType		= GL_SHORT;
			attribChanel	= VertexBuffer::vcTexcoord;
			size			= 2;
			break;
		case VertexBuffer::vcInt16Texcoord2:
			dataType		= GL_SHORT;
			attribChanel	= VertexBuffer::vcTexcoord2;
			size			= 2;
			break;
		case VertexBuffer::vc4Int8BoneIndices:
			dataType		= GL_BYTE;
			attribChanel	= VertexBuffer::vc4FloatBoneIndices;
			size			= 4;
			break;
		case VertexBuffer::vc4FloatBoneIndices:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vc4FloatBoneIndices;
			size			= 4;
			break;
		case VertexBuffer::vc4BoneWeights:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vc4BoneWeights;
			size			= 4;
			break;
		case VertexBuffer::vc2BoneWeights:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vc2BoneWeights;
			size			= 2;
			break;
		case VertexBuffer::vcColor:
			dataType		= GL_FLOAT;
			attribChanel	= VertexBuffer::vcColor;
			size			= 4;
			break;
		}
#ifdef _DEBUG
		VertexBuffer::VertexComponent vcAttrib = (VertexBuffer::VertexComponent)attribChanel;//for debugging
#endif
		
		if(!program->hasVertexAttrib(attribChanel))
			continue;

		if(attribChanel >= 0)
		{
			enableClientState( attribChanel );

			int vcOffset = (int)pVB->getComponentOffset(vc);
			const void * dataLocation = (const void *)((uintptr_t)buffer + vcOffset);

			glVertexAttribPointer(attribChanel,	size, dataType, normalized, pVB->getVertexSize(), dataLocation);

			CHECK_GL_ERROR;
		}
	}
}

void Render::renderIndexBuffer(RenderData::IndexBuffer * pIB, tuple2i range, int forceCullFace)
{
	ASSERT(pIB);
	ASSERT(range.y <= (int)pIB->getIndicesNum());

	//create gl index buffer if it's not created yet
	IndexBuffer * glIB = TYPE_CAST<IndexBuffer*>(pIB);

	if(mLastVB)
	{
		//int firstWrongIndex = glIB->checkIndices(mLastVB->getVertsNum() - 1);
		//if(firstWrongIndex >= 0)
		{
		//	ASSERT(false);
		}
	}

	if(!glIB->isCreated() && glIB->getStorageType() != RenderData::IBuffer::stCPUMemory)
	{
		glIB->create();
	}

	//bind gl index buffer
	void * indsBuffer = pIB->getIndexAddr(range.x);
	if(glIB->isCreated())
	{
		indsBuffer = 0;
		glIB->bind();
	}
	else
	{
		IndexBuffer::Unbind();
	}

	//setup cull face
	
	int cullFace = pIB->getPolyOri();
	if(forceCullFace >= 0)
		cullFace = forceCullFace;

	if(mCullFace != cullFace)
	{
		if(cullFace != IndexBuffer::poNone)
		{
 			enableState(GL_CULL_FACE);
			glFrontFace(cullFace);
		}
		else
 			enableState(GL_CULL_FACE, false);

		mCullFace = cullFace;
	}

	CHECK_GL_ERROR;

	//draw elements
	int indsNum = range.y - range.x;
	glDrawElements( pIB->getPolyType(), 
					indsNum, 
					pIB->getIndexSize() == RenderData::IndexBuffer::Index16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 
					indsBuffer);

	CHECK_GL_ERROR;

	++mStats.mDrawCallsNum;
	mStats.mTrianglesNum += indsNum/3;
}

RenderData::IndexBuffer * Render::createIndexBuffer(int indsNum, RenderData::IndexBuffer::IndexSize indexSize)
{
	IndexBuffer * ib = new IndexBuffer(indsNum, indexSize);
	ib->setPool(&mContextObjects);
	return ib;
}

RenderData::IndexBuffer * Render::createIndexBuffer(int indsNum)//TODO: remove this method - need to specify index size explicitly
{
	IndexBuffer * ib = new IndexBuffer(indsNum, RenderData::IndexBuffer::Index32);
	ib->setPool(&mContextObjects);
	return ib;
}

RenderData::VertexBuffer * Render::createVertexBuffer(int vertType, int vertsNum)
{
	VertexBuffer * vb = new VertexBuffer(vertType, vertsNum);
	vb->setPool(&mContextObjects);
	return vb;
}

ITexture * Render::createTexture()
{
	Texture * texture = new Texture();
	texture->setPool(&mContextObjects);
	return texture;
}

IProgram * Render::createProgram()
{
	Program * program = new Program();
	program->setPool(&mContextObjects);
	return program;
}

IFrameBuffer * Render::createFrameBuffer(int width, int height, int flags)
{
	FrameBuffer * fb = new FrameBuffer(width, height, flags);
	fb->setPool(&mContextObjects);
	return fb;
}

} //namespace GL {

} //namespace Squirrel {
