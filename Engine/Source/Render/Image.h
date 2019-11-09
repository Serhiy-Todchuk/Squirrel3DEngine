#pragma once

#include "macros.h"
#include <vector>
#include <Common/types.h>
#include <Common/macros.h>
#include <Common/tuple.h>
#include <Math/vec3.h>

namespace Squirrel {

class Data;

namespace RenderData { 

class SQRENDER_API Image
{
public:

	//Matches OpenGL's consts
	//TODO:? add R and RG formats (and RGTC compression type)
	enum Format
	{
		UnknownFormat	= 0,
		Alpha			= 0x1906,
		RGB				= 0x1907,
		RGBA			= 0x1908,
		BGR				= 0x80E0,
		BGRA			= 0x80E1,
		Luminance		= 0x1909,
		LuminanceAlpha	= 0x190A
	};

	//compression
	enum Compression
	{
		Uncompressed = 0,
		DXT1,
		DXT3,
		DXT5,
		LATC,
		LATCSigned,
		PVR2BPP,
		PVR4BPP
	};

	//Matches OpenGL's consts
	enum DataType
	{
		UnknownDataType	= 0,
		Int8			= 0x1401,
		Int16			= 0x1403,
		Int32			= 0x1404,
		Float16			= 0x140B,
		Float32			= 0x1406
	};

	enum ContentType
	{
		Other = 0,
		Colors,
		Vectors
	};

	enum DepthType
	{
		Volume = 0,
		Array
	};

	struct Level
	{
		tuple3i dimensions;
		uint32 size;
		byte * data;
	};

	typedef std::vector<Level> LEVELS_VEC;

public:
	//ctors
	Image(void);
	Image(uint32 w, uint32 h, uint32 d, DataType dataType, Format format);
	Image(	uint32 w, uint32 h, uint32 d, 
			DataType dataType, Format format,
			Compression compression,
			int levels, bool cubemap = false);
	Image(Data * data);

	//destr
	~Image(void);

	static float GetInvCompressionRatio(Compression compression, uint32 componentsNum);

	static const char * const GetNativeFileSign();

	//accessors
	inline bool		isNative()		const { return mNative; }
	inline DataType	getDataType()	const { return mDataType; }
	inline Format	getFormat()		const { return mFormat; }
	inline uint32	getHeight()		const { return mHeight; }
	inline uint32	getWidth()		const { return mWidth; }
	inline uint32	getDepth()		const { return mDepth; }

	inline byte*	getData()		
		{ return mData[0].data; }

	inline const Level&	getLevel(uint32 level) const 
	{
		return mData[level];
	}

	inline const Level&	getLevel(uint32 level, uint32 face) const 
	{
		return mData[level * mFaces + face]; 
	}

	inline Level&	getLevel(uint32 level, uint32 face) 
	{
		return mData[level * mFaces + face]; 
	}

	inline uint32		getLevelsNum() const
		{ return mLevels; }

	inline uint32		getFacesNum() const
		{ return mFaces; }

	inline Compression	getCompression() const 
		{ return mCompression; }

	inline void setContentType(ContentType ct) { mContentType = ct; }
	inline ContentType getContentType() { return mContentType; }

	byte * getPixel(uint32 w, uint32 h, uint32 d = 0);
	byte * getPixelFromLevel(uint32 level, uint32 w, uint32 h, uint32 d = 0);

	uint32 getComponentSize();
	uint32 getComponentsNum();

	bool buildMipMaps();

	Image * buildNormalHeightMapFromHeight(float scale);
	Image * buildNormalHeightMapFromNormals();
	Image * downsample(uint32 level, int face = 0);
	Image * downsampleNHM(uint32 level, int face = 0);
	
	Image * resize(uint32 newWidth, uint32 newHeight);

	bool load(Data * data);
	bool save(Data * data);

	void swapYZ();

private:

	void genEmptyFaces(uint32 w, uint32 h, uint32 d);
	uint32 calcCompressedDataSize(uint32 w, uint32 h, uint32 d);

	Math::vec3 getNormal(int i, int j);

private:

	bool		mNative;

	//hint for textures how to behave with this image (especially while interpolation)
	//so maybe need to rename it to ErpType
	ContentType	mContentType;

    Format		mFormat;

	DataType	mDataType;

	Compression	mCompression;

	DepthType	mDepthType;

	uint32		mWidth;

	uint32		mHeight;

    uint32		mDepth;

	uint32		mFaces;

    uint32		mLevels;

	LEVELS_VEC	mData;

	static int sDownsampleLimit;

};

}//namespace RenderData { 

}//namespace Squirrel {

