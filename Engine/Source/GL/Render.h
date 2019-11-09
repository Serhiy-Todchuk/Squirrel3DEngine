#pragma once

#include <Render/IRender.h>
#include "Texture.h"
#include <Common/IDMap.h>

namespace Squirrel {

namespace GL { 

using namespace Math;
using namespace Render;
using namespace RenderData;

class SQOPENGL_API Render:
	public IRender
{
	static void enableClientState(uint state, bool enable = true);
	static void enableState(uint state, bool enable = true);

	vec4 mColor;

	mat4 mProjectionMatrix;
	mat4 mModelViewMatrix;
	mat4 mMVPMatrix;
	mat3 mNormalMatrix;

	RenderData::VertexBuffer * mLastVB;

	int mCullFace;

	BlendMode mBlendMode;
	bool mColorWrite;
	DepthTestMode mDepthTest;
	bool mDepthWrite;

	float mAlphaTest;
	vec2 mPolygonOffset;

public:
	Render();
	virtual ~Render();

	virtual void setColor(vec4 color);
	virtual void setViewport(int x, int y, int width, int height);
	virtual void clear(bool color, bool depth, bool stencil = false);
	virtual void present();
	virtual void flush(bool waitUntilDone);

	virtual void enableDepthTest(DepthTestMode mode = depthCompareOrEqual);
	virtual void enableDepthWrite(bool enable);

	virtual void enableColorWrite(bool enable);

	//1.0f - test always fails (object fully transparent), 
	//0.0f - test always success (turned off alpha test)
	virtual void setAlphaTestValue(float alpha);
	virtual void setBlendMode(BlendMode mode);

	virtual void activeTextureUnit(int unit = 0);
	virtual int getActiveTextureUnit();

	virtual void setRasterizationMode(RasterizationMode mode);

	virtual void enablePolygonOffset(bool enable, float factor, float units);

	virtual void setProjection(const mat4& projection);
	virtual void setTransform(const mat4& transform, IProgram * program = NULL);

	virtual bool checkError(const char_t * message);

	virtual void obtainDepthBuffer(float * dstCPUBuff);

	//VB operations
	virtual void setupVertexBuffer(RenderData::VertexBuffer * pVB);

	//IB operations
	virtual void renderIndexBuffer(RenderData::IndexBuffer * pIB, tuple2i range, int forceCullFace = -1);

	//create operations
	virtual ITexture * createTexture();
	virtual IProgram * createProgram();
	virtual IFrameBuffer * createFrameBuffer(int width, int height, int flags);
	virtual RenderData::VertexBuffer * createVertexBuffer(int vertType, int vertsNum);
	virtual RenderData::IndexBuffer * createIndexBuffer(int indsNum);
	virtual RenderData::IndexBuffer * createIndexBuffer(int indsNum, RenderData::IndexBuffer::IndexSize indexSize);

};


} //namespace GL {

} //namespace Squirrel {
