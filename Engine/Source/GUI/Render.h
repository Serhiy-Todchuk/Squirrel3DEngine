#pragma once

#include "Font.h"
#include <Render/IRender.h>
#include <Resource/Mesh.h>
#include <Resource/Program.h>
#include <Common/common.h>
#include <Math/vec4.h>
#include <Common/tuple.h>
#include <memory>

namespace Squirrel {
namespace GUI { 

class SQGUI_API Render: 
	public FontRender
{
public:

	static const tuple2i TEXT_OFFSET;
	static const int MAX_VERTS_NUM_PER_BATCH = 2048;
	static const int MAX_INDS_NUM_PER_BATCH = 4096;

	enum Size
	{
		sizeSmall = 0,
		sizeNormal,
		sizeBig,
		sizesNum
	};

private:

	Render();

public:

	~Render();

	static Render& Instance();

	void drawFoldout(tuple2i pos, tuple2i size, bool expanded);
	void drawBackPlane(tuple2i pos, tuple2i size);
	void drawSelectPlane(tuple2i pos, tuple2i size);
	void drawPlane(tuple2i pos, tuple2i size, bool pushed);
	void drawBorder(tuple2i pos, tuple2i size, bool pushed);
	void drawText(tuple2i pos, const std::string& text, Size fSize);

	void drawRect(tuple2i pos, tuple2i size, Math::vec4 color);
	void drawBorder(tuple2i pos, tuple2i size, Math::vec2 depth, Math::vec2 width);

	Math::vec4 getClipLines() { return mClipLines; }
	void enableClipLines(float up, float down, float left, float right); 
	void enableClipLines(Math::vec4 clipLines); 
	void disableClipLines(); 

	void finish();
	void flush();
	void batchMesh(Resource::Mesh * mesh);

	inline float	getDepth()			{ return mDepth; }
	inline void		setDepth(float d)	{ mDepth = d; }

	inline Font *	getFont(Size fSize)					{ return mFonts[(int)fSize].get(); }
	inline void	setFont(Size fSize, Font * font)	{ mFonts[(int)fSize].reset(font); }

private:

	struct BatchMesh
	{
		Resource::Texture * texture;
		Resource::Mesh * mesh;
		int verts;
		int inds;
	};

	typedef std::map<Resource::Texture *, BatchMesh> BATCHES_MAP;

private:

	virtual void drawChar( Math::vec3 vertices[VERTS_PER_CHAR], Math::vec2 texcoords[VERTS_PER_CHAR] );

	BatchMesh& getBatchMesh(Resource::Texture * texture);

	void flushBatch(Resource::Texture * texture);
	void flushBatch(BatchMesh& batch);

	bool isVisible(tuple2i pos, tuple2i size);

	void clipVerts(RenderData::VertexBuffer * vb, tuple2i range);

	void incrDepth();

private:

	BATCHES_MAP mBatches;

	Math::vec4 mBackColor;//TODO: to make it ColorOrTexture
	Math::vec4 mColor;
	Math::vec4 mTextColor;
	Math::vec4 mBorderColor;
	Math::vec4 mSelectColor;

	Math::vec4 mColorOffset;

	Math::vec4 mClipLines;//TODO: rename to mClipBounds
	bool		mClipping;

	std::auto_ptr<Font> mFonts[sizesNum];

	Squirrel::Render::IRender * mRender;//pre-cached render

	Resource::Mesh * mRectMesh;
	Resource::Mesh * mBorderMesh;

	Resource::Texture * mSkinTexture;

	Resource::Program * mProgram;
	Squirrel::Render::IProgram *	mProgramColored;
	Squirrel::Render::IProgram *	mProgramTextured;
	Squirrel::Render::IProgram *	mProgramForText;

	BatchMesh * mCurrentTextBatch;

	bool mAutoincrementDepth;

	float mDepth;//z-coord
};

}//namespace GUI { 
}//namespace Squirrel {