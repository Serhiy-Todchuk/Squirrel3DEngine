#include "Render.h"
#include "Bounds2D.h"
#include <Render/ITexture.h>
#include <Resource/ResourceManager.h>

namespace Squirrel {
namespace GUI { 

using namespace Math;
using namespace RenderData;

const tuple2i Render::TEXT_OFFSET = tuple2i(2,1);

const int BORDER_MESH_VERTS	= 8;
const int RECT_MESH_VERTS	= 4;

bool useHardwareClipping = true;

IBuffer::StorageType bufferStorageType = IBuffer::stGPUDynamicMemory;

Render::Render() 
{
	//pre-cache render
	mRender = Squirrel::Render::IRender::GetActive();

	mDepth = -50;

	mSkinTexture = NULL;

	disableClipLines();

	mCurrentTextBatch = NULL;

	mAutoincrementDepth = true;

	mProgram = Resource::ProgramStorage::Active()->add("GUI/GUI.glsl");

	if(mProgram)
	{
		std::string commonParams = useHardwareClipping ? "CLIPPING;" : "";

		mProgramColored		= mProgram->getRenderProgram(commonParams + "VERTEX_COLOR;");
		mProgramTextured	= mProgram->getRenderProgram(commonParams + "TEXTURE;" + "VERTEX_COLOR;");
		mProgramForText		= mProgram->getRenderProgram(commonParams + "TEXTURE;");
	}

	//
	// define default colors
	//

	mBackColor		= vec4(0.9f, 1.0f, 0.9f, 0.5f);
	mColor			= vec4(0.1f, 0.5f, 0.3f, 0.7f);
	mTextColor		= vec4(0.1f, 0.0f, 0.1f, 1.0f);
	mBorderColor	= vec4(0.2f, 0.2f, 0.2f, 0.9f);
	mSelectColor	= vec4(0.5f, 0.3f, 0.7f, 0.8f);

	mColorOffset	= vec4(0.1f, 0.1f, 0.1f, 0.0f);

	//
	// setup rect mesh
	//

	mRectMesh = new Resource::Mesh();

	//init vertex buffer
	VertexBuffer * vb = mRectMesh->createVertexBuffer(useHardwareClipping ? VT_GUI_CLIPPED : VT_GUI, RECT_MESH_VERTS);
	vb->setStorageType(VertexBuffer::stCPUMemory);

	//init index buffer
	IndexBuffer * ib = mRectMesh->createIndexBuffer(6);
	ib->setPolyOri(IndexBuffer::poNone);
	ib->setPolyType(IndexBuffer::ptTriangles);

	ib->setIndex(0, 0);	//   vertices:	|	  indices:
	ib->setIndex(1, 3);	//	  0___3		|	0___1	    0
	ib->setIndex(2, 1);	//	  |  /|		|	|  / 	   /|
	ib->setIndex(3, 3);	//	  |/__|		|	|/	 	 /__|
	ib->setIndex(4, 2);	//	  1   2		|	2	 	2   1
	ib->setIndex(5, 1);	//				|

	//
	// setup border mesh
	//

	mBorderMesh = new Resource::Mesh();

	//init vertex buffer
	vb = mBorderMesh->createVertexBuffer(useHardwareClipping ? VT_GUI_CLIPPED : VT_GUI, BORDER_MESH_VERTS);
	vb->setStorageType(VertexBuffer::stCPUMemory);

	//init index buffer
	ib = mBorderMesh->createIndexBuffer(24);
	ib->setPolyOri(IndexBuffer::poNone);
	ib->setPolyType(IndexBuffer::ptTriangles);

	//vertices layout
	//	0_______3
	//	| 4___7	|
	//	| |   |	|
	//	| |___|	|
	//	|_5___6_|
	//	1       2

	//vertices to indices mapping
	//   vertices:	|	  indices:
	//	  0___3		|	0___1	    0
	//	  |  /|		|	|  / 	   /|
	//	  |/__|		|	|/	 	 /__|
	//	  1   2		|	2	 	2   1
	//				|

	int ind = 0;

	//left quad
	ib->setIndex(ind++, 0);
	ib->setIndex(ind++, 4);
	ib->setIndex(ind++, 1);
	ib->setIndex(ind++, 4);
	ib->setIndex(ind++, 5);
	ib->setIndex(ind++, 1);

	//bottom quad
	ib->setIndex(ind++, 5);
	ib->setIndex(ind++, 6);
	ib->setIndex(ind++, 1);
	ib->setIndex(ind++, 6);
	ib->setIndex(ind++, 2);
	ib->setIndex(ind++, 1);

	//right quad
	ib->setIndex(ind++, 7);
	ib->setIndex(ind++, 3);
	ib->setIndex(ind++, 6);
	ib->setIndex(ind++, 3);
	ib->setIndex(ind++, 2);
	ib->setIndex(ind++, 6);

	//top quad
	ib->setIndex(ind++, 0);
	ib->setIndex(ind++, 3);
	ib->setIndex(ind++, 4);
	ib->setIndex(ind++, 3);
	ib->setIndex(ind++, 7);
	ib->setIndex(ind++, 4);
}

Render::~Render() 
{
}

Render& Render::Instance()
{
	static Render instance;
	return instance;
}

Render::BatchMesh& Render::getBatchMesh(Resource::Texture * texture)
{
	BATCHES_MAP::iterator it = mBatches.find(texture);
	if(it != mBatches.end())
	{
		return it->second;
	}
	else
	{
		//
		// setup batch mesh
		//

		BatchMesh batch;

		batch.mesh = new Resource::Mesh();

		//init vertex buffer
		VertexBuffer * vb = batch.mesh->createVertexBuffer(useHardwareClipping ? VT_GUI_CLIPPED : VT_GUI, MAX_VERTS_NUM_PER_BATCH);
		vb->setStorageType(bufferStorageType);

		//init index buffer
		IndexBuffer * ib = batch.mesh->createIndexBuffer(MAX_INDS_NUM_PER_BATCH);
		ib->setStorageType(bufferStorageType);
		ib->setPolyOri(IndexBuffer::poNone);
		ib->setPolyType(IndexBuffer::ptTriangles);

		batch.verts = 0;
		batch.inds = 0;

		batch.texture = texture;

		mBatches[texture] = batch;

		return mBatches[texture];
	}
}

void Render::drawFoldout(tuple2i pos, tuple2i size, bool expanded)
{
	if(mClipping)
	{
		if(!isVisible(pos, size)) 
			return;
	}

	RenderData::VertexBuffer * vb = mRectMesh->getVertexBuffer();

	vec3 pos1( (float)pos.x, (float)pos.y, mDepth );
	vec3 pos2( (float)pos.x, (float)pos.y + size.y, mDepth );
	vec3 pos3( (float)pos.x + size.x, (float)pos.y + size.y, mDepth );
	vec3 pos4( (float)pos.x + size.x, (float)pos.y, mDepth );

	vec2 tex1( 0, 0 );
	vec2 tex2( 0, 1 );
	vec2 tex3( 1, 1 );
	vec2 tex4( 1, 0 );

	if(expanded)
	{
		pos2.x = (float)pos.x + 0.5f * size.x;
		pos3.x = (float)pos.x + 0.5f * size.x;
		tex2.x = 0.5f;
		tex3.x = 0.5f;
	}
	else
	{
		pos3.y = (float)pos.y + 0.5f * size.y;
		pos4.y = (float)pos.y + 0.5f * size.y;
		tex3.y = 0.5f;
		tex4.y = 0.5f;
	}

    // 1
	vb->setComponent<VertexBuffer::vcPosition>( 0, pos1 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 0, tex1 );
	vb->setComponent<VertexBuffer::vcColor>( 0, mTextColor );

    // 2
	vb->setComponent<VertexBuffer::vcPosition>( 1, pos2 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 1, tex2 );
	vb->setComponent<VertexBuffer::vcColor>( 1, mTextColor );

    // 3
	vb->setComponent<VertexBuffer::vcPosition>( 2, pos3 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 2, tex3 );
	vb->setComponent<VertexBuffer::vcColor>( 2, mTextColor );

    // 4
	vb->setComponent<VertexBuffer::vcPosition>( 3, pos4 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 3, tex4 );
	vb->setComponent<VertexBuffer::vcColor>( 3, mTextColor );

	//render rect
	batchMesh(mRectMesh);

	incrDepth();
}

void Render::drawBackPlane(tuple2i pos, tuple2i size)
{
	drawRect(pos, size, mBackColor);
}

void Render::drawSelectPlane(tuple2i pos, tuple2i size)
{
	drawRect(pos, size, mSelectColor);
}

void Render::drawPlane(tuple2i pos, tuple2i size, bool pushed)
{
	vec4 color = mColor;
	if(pushed) color -= vec4(0.1f, 0.1f, 0.1f, 0.0f);
	drawRect(pos, size, color);
}

void Render::drawBorder(tuple2i pos, tuple2i size, bool pushed)
{
	mRender->setColor(mTextColor);

	drawBorder(pos, size, vec2(0, 0), vec2(1, 1));
}

bool Render::isVisible(tuple2i pos, tuple2i size)
{
	Bounds2D elemBounds(pos, pos + size);
	Bounds2D clippingBounds(tuple2i(mClipLines.z, mClipLines.x), tuple2i(mClipLines.w, mClipLines.y));

	return elemBounds.intersects(clippingBounds);
}

void Render::drawText(tuple2i pos, const std::string& text, Size fSize)
{
	mRender->setColor(mTextColor);

	pos += TEXT_OFFSET;
	Font * font = getFont(fSize);

	if(mClipping)
	{
		tuple2i size;
		size.x = font->getStrWidth(text.c_str());
		size.y = font->getCharHeight();

		if(!isVisible(pos, size)) return;
	}

	BatchMesh& batch = getBatchMesh(font->getTexture());

	mCurrentTextBatch = &batch;
	font->setFontRender(this);

	font->setDepth(mDepth);
	font->drawText(pos.x, pos.y, text.c_str());

	font->setFontRender(NULL);
	mCurrentTextBatch = NULL;

	incrDepth();
}

void Render::drawRect(tuple2i pos, tuple2i size, Math::vec4 color)
{
	if(mClipping)
	{
		if(!isVisible(pos, size)) 
			return;
	}

	RenderData::VertexBuffer * vb = mRectMesh->getVertexBuffer();

	vec3 pos1( (float)pos.x, (float)pos.y, mDepth );
	vec3 pos2( (float)pos.x, (float)pos.y + size.y, mDepth );
	vec3 pos3( (float)pos.x + size.x, (float)pos.y + size.y, mDepth );
	vec3 pos4( (float)pos.x + size.x, (float)pos.y, mDepth );

	vec2 tex1( 0, 0 );
	vec2 tex2( 0, 1 );
	vec2 tex3( 1, 1 );
	vec2 tex4( 1, 0 );

    // 1
	vb->setComponent<VertexBuffer::vcPosition>( 0, pos1 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 0, tex1 );
	vb->setComponent<VertexBuffer::vcColor>( 0, color );

    // 2
	vb->setComponent<VertexBuffer::vcPosition>( 1, pos2 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 1, tex2 );
	vb->setComponent<VertexBuffer::vcColor>( 1, color );

    // 3
	vb->setComponent<VertexBuffer::vcPosition>( 2, pos3 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 2, tex3 );
	vb->setComponent<VertexBuffer::vcColor>( 2, color );

    // 4
	vb->setComponent<VertexBuffer::vcPosition>( 3, pos4 );
	vb->setComponent<VertexBuffer::vcTexcoord>( 3, tex4 );
	vb->setComponent<VertexBuffer::vcColor>( 3, color );

	//render rect
	batchMesh(mRectMesh);

	incrDepth();
}

void Render::drawBorder(tuple2i pos, tuple2i size, Math::vec2 depth, Math::vec2 width)
{
	if(mClipping)
	{
		if(!isVisible(pos, size)) 
			return;
	}

	//vertices layout
	//	0_______3
	//	| 4___7	|
	//	| |   |	|
	//	| |___|	|
	//	|_5___6_|
	//	1       2

	float outerDepth = mDepth + depth.x;
	float innerDepth = mDepth + depth.y;

	RenderData::VertexBuffer * vb = mBorderMesh->getVertexBuffer();

	vec2 topLft1( (float)pos.x, (float)pos.y );
	vec2 botRht1( topLft1 + vec2( (float)size.x, (float)size.y ) );
	vec2 topLft2( topLft1 + width );
	vec2 botRht2( botRht1 - width );

	vec3 positions[BORDER_MESH_VERTS];
	positions[0] = vec3( topLft1.x, topLft1.y, outerDepth );
	positions[1] = vec3( topLft1.x, botRht1.y, outerDepth );
	positions[2] = vec3( botRht1.x, botRht1.y, outerDepth );
	positions[3] = vec3( botRht1.x, topLft1.y, outerDepth );
	positions[4] = vec3( topLft2.x, topLft2.y, innerDepth );
	positions[5] = vec3( topLft2.x, botRht2.y, innerDepth );
	positions[6] = vec3( botRht2.x, botRht2.y, innerDepth );
	positions[7] = vec3( botRht2.x, topLft2.y, innerDepth );

	for(int i = 0; i < BORDER_MESH_VERTS; ++i)
	{
		vb->setComponent<VertexBuffer::vcPosition>( i, positions[i] );
		vb->setComponent<VertexBuffer::vcColor>( i, mBorderColor );
	}

	//render rect
	batchMesh(mBorderMesh);

	incrDepth();
}

void Render::drawChar( Math::vec3 vertices[VERTS_PER_CHAR], Math::vec2 texcoords[VERTS_PER_CHAR] )
{
	if(mCurrentTextBatch == NULL) return;

	IndexBuffer * ib = mCurrentTextBatch->mesh->getIndexBuffer();
	VertexBuffer * vb = mCurrentTextBatch->mesh->getVertexBuffer();

	if((mCurrentTextBatch->verts + VERTS_PER_CHAR >= vb->getVertsNum()) ||
		(mCurrentTextBatch->inds + INDS_PER_CHAR >= ib->getIndicesNum())) 
		flushBatch(*mCurrentTextBatch);

	int v	= mCurrentTextBatch->verts;
	mCurrentTextBatch->verts += VERTS_PER_CHAR;

	for(int j = 0; j < VERTS_PER_CHAR; ++j)
	{
		vb->setComponent<VertexBuffer::vcPosition>(v + j, vertices[j]);
		vb->setComponent<VertexBuffer::vcTexcoord>(v + j, texcoords[j]);
		vb->setComponent<VertexBuffer::vcColor>(v + j, mTextColor);
	}

	if(mClipping || useHardwareClipping)
	{
		clipVerts(vb, tuple2i(v, mCurrentTextBatch->verts));
	}

	int i	= mCurrentTextBatch->inds;
	mCurrentTextBatch->inds += INDS_PER_CHAR;

	ib->setIndex(i++, v + 0);	//   vertices:	|	  indices:
	ib->setIndex(i++, v + 3);	//	  0___3		|	0___1	    0
	ib->setIndex(i++, v + 1);	//	  |  /|		|	|  / 	   /|
	ib->setIndex(i++, v + 3);	//	  |/__|		|	|/	 	 /__|
	ib->setIndex(i++, v + 2);	//	  1   2		|	2	 	2   1
	ib->setIndex(i++, v + 1);	//				|
}

void Render::clipVerts(RenderData::VertexBuffer * vb, tuple2i range)
{
	range.x = Math::clamp(range.x, 0, (int)vb->getVertsNum());
	range.y = Math::clamp(range.y, 0, (int)vb->getVertsNum());

	if(useHardwareClipping)
	{
		for(int j = range.x; j < range.y; ++j)
		{
			vb->setComponent<VertexBuffer::vcTangentBinormal>(j, mClipLines);
		}
	}
	else
	{
		const float posEpsilon = 0.01f;

		for(int j = range.x; j < range.y; ++j)
		{
			Math::vec3 pos = vb->getComponent<VertexBuffer::vcPosition>(j);
			Math::vec2 tex = vb->getComponent<VertexBuffer::vcTexcoord>(j);

			Math::vec3 newPos = pos;
			Math::vec2 newTex = tex;

			int jMod = j % 4;
			int otherPosIndex = j + (jMod < 2 ? 2 : -2);
			Math::vec3 otherPos = vb->getComponent<VertexBuffer::vcPosition>(otherPosIndex);
			Math::vec2 otherTex = vb->getComponent<VertexBuffer::vcTexcoord>(otherPosIndex);

			//mClipLines -> up, down, left, right
			if(pos.y < mClipLines.x)	newPos.y = mClipLines.x;
			if(pos.y > mClipLines.y)	newPos.y = mClipLines.y;
			if(pos.x < mClipLines.z)	newPos.x = mClipLines.z;
			if(pos.x > mClipLines.w)	newPos.x = mClipLines.w;

			Math::vec3 diffPosTemplate	= otherPos - pos;
			Math::vec3 diffPos			= newPos - pos;
			Math::vec3 diffFactor		= diffPos.div(diffPosTemplate);

			if(fabsf(diffPos.x) > posEpsilon)
			{
				newTex.x = Math::lerp(tex.x, otherTex.x, diffFactor.x);
			}
			if(fabsf(diffPos.y) > posEpsilon)
			{
				newTex.y = Math::lerp(tex.y, otherTex.y, diffFactor.y);
			}

			vb->setComponent<VertexBuffer::vcPosition>(j, newPos);
			vb->setComponent<VertexBuffer::vcTexcoord>(j, newTex);
		}
	}
}

void Render::batchMesh(Resource::Mesh * mesh)
{
	VertexBuffer * srcVB = mesh->getVertexBuffer();

	if(mClipping || useHardwareClipping)
	{
		clipVerts(srcVB, tuple2i(0, srcVB->getVertsNum()));
	}

	IndexBuffer * srcIB = mesh->getIndexBuffer();

	BatchMesh& batch = getBatchMesh(mSkinTexture);

	VertexBuffer * dstVB = batch.mesh->getVertexBuffer();
	IndexBuffer * dstIB = batch.mesh->getIndexBuffer();

	if(srcVB->getVertType() != srcVB->getVertType()) ASSERT(false);
	if(srcIB->getIndexSize() != srcIB->getIndexSize()) ASSERT(false);

	int freeVerts = MAX_VERTS_NUM_PER_BATCH - batch.verts;
	if(srcVB->getVertsNum() >= freeVerts)	
		flushBatch(batch);

	int freeInds = MAX_INDS_NUM_PER_BATCH - batch.inds;
	if(srcIB->getIndicesNum() >= freeInds)	
		flushBatch(batch);

	memcpy(dstVB->getVertexAddr(batch.verts), srcVB->getVerts(), 
		srcVB->getVertexSize() * srcVB->getVertsNum());

	for(int i = 0; i < srcIB->getIndicesNum(); ++i)
	{
		dstIB->setIndex(batch.inds + i, batch.verts + srcIB->getIndex(i));
	}

	batch.verts += srcVB->getVertsNum();
	batch.inds  += srcIB->getIndicesNum();
}

void Render::finish()
{
	if(mAutoincrementDepth)
	{
		mDepth = -50;
	}

	Render::Instance().disableClipLines();

	flush();
}

void Render::flush()
{
	mRender->setColor(mTextColor);

	if(mProgramForText != NULL)
	{
		mProgramForText->bind();
		mRender->getUniformsPool().fetchUniforms(mProgramForText);
	}
	for(int i = 0; i < sizesNum; ++i)
	{
		mFonts[i].get()->flush();
	}

	mRender->setColor(vec4(1,1,1,1));
	for(BATCHES_MAP::iterator it = mBatches.begin(); it != mBatches.end(); ++it)
	{
		flushBatch(it->second);
	}
}

void Render::flushBatch(Resource::Texture * texture)
{
	BATCHES_MAP::iterator it = mBatches.find(texture);
	if(it != mBatches.end())
	{
		flushBatch(it->second);
	}
}

void Render::flushBatch(BatchMesh& batch)
{
	if(batch.verts == 0 || batch.inds == 0) return;
	
	if(batch.texture == NULL)
	{
		if(mProgramColored != NULL)
		{
			mProgramColored->bind();
			mRender->getUniformsPool().fetchUniforms(mProgramColored);
		}
		else 
			Squirrel::Render::ITexture::Unbind(0); 
	}
	else
	{
		if(mProgramTextured != NULL)
		{
			mProgramTextured->bind();
			mRender->getUniformsPool().fetchUniforms(mProgramTextured);
		}
		batch.texture->getRenderTexture()->bind();
	}

	VertexBuffer * vb = batch.mesh->getVertexBuffer();
	vb->update(0, batch.verts * vb->getVertexSize());

	IndexBuffer * ib = batch.mesh->getIndexBuffer();
	ib->update(0, batch.inds * ib->getIndexSize());

	mRender->setupVertexBuffer(batch.mesh->getVertexBuffer());
	mRender->renderIndexBuffer(batch.mesh->getIndexBuffer(), tuple2i(0, batch.inds));

	batch.verts = 0;
	batch.inds = 0;
}

void Render::incrDepth()
{
	if(mAutoincrementDepth)
	{
		mDepth += 0.1f;
	}
}

void Render::enableClipLines(float up, float down, float left, float right)
{
	mClipping = true;
	mClipLines = vec4(up, down, left, right);
}

void Render::enableClipLines(vec4 clipLines)
{
	enableClipLines(clipLines.x, clipLines.y, clipLines.z, clipLines.w);
}

void Render::disableClipLines()
{
	mClipping = false;
	mClipLines = vec4(0, 9999, 0, 9999);
}

}//namespace GUI { 
}//namespace Squirrel {
