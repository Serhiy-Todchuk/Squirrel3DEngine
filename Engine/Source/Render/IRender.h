#pragma once

#include "Image.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ITexture.h"
#include "IProgram.h"
#include "IFrameBuffer.h"
#include "IContextObject.h"
#include <Common/types.h>
#include <Common/Window.h>
#include <Math/mathTypes.h>
#include <string>
#include "Uniform.h"
#include "macros.h"

namespace Squirrel {

namespace Render { 

using namespace Math;

struct RenderStatistics
{
	int mDrawCallsNum;
	int mBatchesNum;
	int mTrianglesNum;
	int mVerticesNum;
	int mTextureSwitchesNum;
	int mStateSwitchesNum;

	void clear()
	{
		mDrawCallsNum		= 0;
		mBatchesNum			= 0;
		mTrianglesNum		= 0;
		mVerticesNum		= 0;
		mTextureSwitchesNum	= 0;
		mStateSwitchesNum	= 0;
	}
};

class SQRENDER_API IRender
{
	static IRender * sActiveRender;
	static tuple2i sScreenSize;

protected:

	IContextObject::OBJECTS_POOL mContextObjects;

	RenderStatistics mStats;

	UniformContainer mUniformsPool;

	Window * mWindow;

	tuple4i mViewport;

public:

	static UniformString sColorUniformName;
	static UniformString sAlphaTestUniformName;
	static UniformString sModelviewMatrixUniformName;
	static UniformString sMVPMatrixUniformName;
	static UniformString sNormalMatrixUniformName;

	enum BlendMode
	{
		blendOff = 0,
		blendAdd,
		blendSprites,
		blendOneMinusAlpha//depend on alpha value
	};

	enum DepthTestMode
	{
		depthTestOff = 0,
		depthCompareOrEqual,
		depthEqual,
	};

	enum RasterizationMode
	{
		rPoint = 0,
		rLine,
		rFill
	};

	IRender();
	virtual ~IRender();

	RenderStatistics& getRenderStatistics() { return mStats; }
	UniformContainer& getUniformsPool() { return mUniformsPool; }

	static IRender * GetActive() { return sActiveRender; }
	void setAsActive() { sActiveRender = this; }

	Window * getWindow() { return mWindow; }
	void setWindow(Window * wnd) { mWindow = wnd; }

	tuple4i getViewport() { return mViewport; }

	virtual void setColor(vec4 color) = 0;
	virtual void setViewport(int x, int y, int width, int height) = 0;
	virtual void clear(bool color, bool depth, bool stencil = false) = 0;
	virtual void present() = 0;
	virtual void flush(bool waitUntilDone) = 0;

	virtual bool checkError(const char_t * message) = 0;

	virtual void enableDepthTest(DepthTestMode mode = depthCompareOrEqual) = 0;
	virtual void enableDepthWrite(bool enable) = 0;

	virtual void enableColorWrite(bool enable) = 0;

	virtual void setRasterizationMode(RasterizationMode mode) = 0;

	//1.0f - test always fails (object fully transparent), 
	//0.0f - test always success (turned off alpha test)
	virtual void setAlphaTestValue(float alpha) = 0;

	virtual void setBlendMode(BlendMode mode) = 0;

	virtual void activeTextureUnit(int unit = 0) = 0;
	virtual int getActiveTextureUnit() = 0;

	virtual void enablePolygonOffset(bool enable, float factor = 4, float units = 4) = 0;

	//transform
	virtual void setProjection(const mat4& projection) = 0;
	virtual void setTransform(const mat4& transform, IProgram * program = NULL) = 0;

	virtual void obtainDepthBuffer(float * dstCPUBuff) = 0;

	//create operations
	virtual ITexture *					createTexture() = 0;
	virtual IProgram *					createProgram() = 0;
	virtual IFrameBuffer *				createFrameBuffer(int width, int height, int flags) = 0;
	virtual RenderData::VertexBuffer *	createVertexBuffer(int vertType, int vertsNum) = 0;
	virtual RenderData::IndexBuffer *	createIndexBuffer(int indsNum) = 0;
	virtual RenderData::IndexBuffer *	createIndexBuffer(int indsNum, RenderData::IndexBuffer::IndexSize indexSize) = 0;

	//VB operations
	virtual void setupVertexBuffer(RenderData::VertexBuffer * pVB) = 0;

	//IB operations
	virtual void renderIndexBuffer(RenderData::IndexBuffer * pIB, tuple2i range, int forceCullFace = -1) = 0;
	
	void renderIndexBuffer(RenderData::IndexBuffer * pIB, int forceCullFace = -1)
	{
		ASSERT(pIB);
		renderIndexBuffer( pIB, tuple2i(0, pIB->getIndicesNum()) );
	}
};


}//namespace Render { 

}//namespace Squirrel {

