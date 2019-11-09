#include "Image.h"

#include <map>
#include <Math/vec3.h>
#include <Common/Data.h>

namespace Squirrel {

namespace RenderData { 

#define IMG_FILE_VERSION 1

struct IMG_FILE_HEADER
{
	uint32 size;
	uint32 version;

	uint32 format;
	uint32 dataType;
	uint32 compression;
	uint32 depthType;
	uint32 contentType;

	uint32 width;
	uint32 height;
	uint32 depth;
	uint32 levels;
	uint32 faces;

	uint32 reserved[5];
};

int Image::sDownsampleLimit = 4;

const char * const Image::GetNativeFileSign()
{
	return "sqim";
}

float Image::GetInvCompressionRatio(Compression compression, uint32 componentsNum)
{
	switch(compression)
	{
	case Uncompressed:
		return 1.0f;
	case DXT1:
		return componentsNum == 3 ? 6.0f : 8.0f;
	case DXT3:
	case DXT5:
		return 4.0f;
	case LATC:
	case LATCSigned:
		return 2.0f;
	case PVR2BPP:
		return componentsNum == 3 ? 12.0f : 16.0f;
	case PVR4BPP:
		return componentsNum == 3 ? 6.0f : 8.0f;
	}
	return 1.0f;
}

Image::Image(void)
{
	mNative		= false;
	mFormat		= UnknownFormat;
    mDataType	= UnknownDataType;
    mWidth		= 0;
    mHeight		= 0;
	mContentType= Colors;
	mCompression= Uncompressed;
	mLevels		= 1;
	mFaces		= 1;
	mDepthType	= Volume;
}

Image::Image(uint32 w, uint32 h, uint32 d, DataType dataType, Format format)
{
	mNative		= false;
	mWidth		= w;
	mHeight		= h;
	mDepth		= d;
	mFormat		= format;
	mDataType	= dataType;
	mCompression= Uncompressed;
	mLevels		= 1;
	mFaces		= 1;
	mDepthType	= Volume;
	mContentType= Colors;

	size_t pixNum	= w * h * d;
	size_t pixSize	= getComponentsNum() * getComponentSize();
	size_t dataSize = pixNum * pixSize;

	Level data;
	data.dimensions = tuple3i(w, h, d);
	data.size		= dataSize;
	data.data		= new byte[dataSize];
	ASSERT( data.data );
	mData.push_back(data);
}

Image::Image(	uint32 w, uint32 h, uint32 d, 
		DataType dataType, Format format,
		Compression compression,
		int levels, bool cubemap)
{
	mNative		= false;
	mWidth		= w;
	mHeight		= h;
	mDepth		= d;
	mFormat		= format;
	mDataType	= dataType;
	mCompression= compression;
	mLevels		= 1;
	mDepthType	= Volume;
	mContentType= Colors;

	//disable cubemap for depth
	cubemap = cubemap && d == 1 && w == h;

	//6 faces for cubemap, otherwise 1
	mFaces = cubemap ? 6 : 1;

	//disable compression for volume textures
	if(d > 1) compression = Uncompressed;

	mLevels = 0;
	if(levels <= 0)
	{
		// downsample image
		while ( w >= sDownsampleLimit && h >= sDownsampleLimit && d >= sDownsampleLimit )
		{
			genEmptyFaces(w, h, d);
			++mLevels;

			w = ( w > 1) ? w >> 1 : 1;
			h = ( h > 1) ? h >> 1 : 1;
			d = ( d > 1) ? d >> 1 : 1;
		}
	}
	else
	{
		for(int i = 0; i < levels; ++i)
		{
			genEmptyFaces(w, h, d);
			++mLevels;

			w = ( w > 1) ? w >> 1 : 1;
			h = ( h > 1) ? h >> 1 : 1;
			d = ( d > 1) ? d >> 1 : 1;
		}
	}
}

Image::Image(Data * data)
{
	mNative		= false;
	ASSERT(load(data));
}

uint32 Image::calcCompressedDataSize(uint32 w, uint32 h, uint32 d)
{
	uint32 componentsNum = getComponentsNum();
	size_t pixNum	= w * h * d;
	size_t pixSize	= componentsNum * getComponentSize();

	switch(mCompression)
	{
	case DXT1:
		ASSERT(componentsNum == 3 || componentsNum == 4);
		return ((w+3)/4) * ((h+3)/4) * 8;
	case DXT3:
		ASSERT(componentsNum == 4);
		return ((w+3)/4) * ((h+3)/4) * 16;
	case DXT5:
		ASSERT(componentsNum == 4);
		return ((w+3)/4) * ((h+3)/4) * 16;
	case LATC:
	case LATCSigned:
		ASSERT(componentsNum == 1 || componentsNum == 2);
		return ((w+3)/4) * ((h+3)/4) * 18 * componentsNum;
	case PVR2BPP:
		ASSERT(componentsNum == 3 || componentsNum == 4);
		return ( Math::maxValue((int)w, 8) * Math::maxValue((int)h, 8) * 2 + 7) / 8;
	case PVR4BPP:
		ASSERT(componentsNum == 3 || componentsNum == 4);
		return ( Math::maxValue((int)w, 8) * Math::maxValue((int)h, 8) * 4 + 7) / 8;
	case Uncompressed:
	default:
		return pixNum * pixSize;
	};
}

void Image::genEmptyFaces(uint32 w, uint32 h, uint32 d)
{
	size_t dataSize = calcCompressedDataSize(w, h, d);
	Level data;

	for(uint32 i = 0; i < mFaces; ++i)
	{
		//create data 
		data.dimensions = tuple3i(w, h, d);
		data.size		= dataSize;
		data.data		= new byte[dataSize];
		ASSERT( data.data );
		mData.push_back(data);
	}
}

Image::~Image(void)
{
	for(uint32 i = 0; i < mData.size(); ++i)
		DELETE_ARR(mData[i].data);
}

uint32 Image::getComponentSize()
{
	switch(mDataType)
	{
	case Int8:
		return 1;
	case Int16:
	case Float16:
		return 2;
	case Int32:
	case Float32:
		return 4;
	default:
		return 0;
	};

	return 0;
}

uint32 Image::getComponentsNum()
{
	switch(mFormat)
	{
	case Alpha:
	case Luminance:
		return 1;
	case LuminanceAlpha:
		return 2;
	case RGB:
	case BGR:
		return 3;
	case RGBA:
	case BGRA:
		return 4;
	default:
		return 0;
	};

	return 0;
}

byte * Image::getPixel(uint32 w, uint32 h, uint32 d)
{
	if(w >= mWidth)		return NULL;
	if(h >= mHeight)	return NULL;
	if(d >= mDepth)		return NULL;

	uint32 index = w + h * (mWidth) + d * (mWidth * mHeight);

	return &mData[0].data[index * getComponentSize() * getComponentsNum()];
}

byte * Image::getPixelFromLevel(uint32 level, uint32 w, uint32 h, uint32 d)
{
	const Level& levelData = getLevel(level);

	if(w >= levelData.dimensions.x)		return NULL;
	if(h >= levelData.dimensions.y)		return NULL;
	if(d >= levelData.dimensions.z)		return NULL;

	uint32 index = w + h * (levelData.dimensions.x) + d * (levelData.dimensions.x * levelData.dimensions.y);

	return &levelData.data[index * getComponentSize() * getComponentsNum()];
}


Image * Image::buildNormalHeightMapFromHeight(float scale)
{
	Image * nhm = new Image(mWidth, mHeight, 1, Int8, RGBA);
	if(nhm == NULL) return NULL;
	nhm->setContentType(Vectors);

    const float oneOver255 = 1.0f / 255.0f;

    for ( uint32 i = 0; i < mWidth; i++ )
	{
        for ( uint32 j = 0; j < mHeight; j++ )
        {
			// get heights
			byte	h	= getPixel(i, j)[0];
			byte	h1	= getPixel(i, (j+1)%mHeight)[0];
			byte	h2	= getPixel((i+1)%mWidth, j)[0];
            
			// convert height values to [0,1] range
            float	c  = h  * oneOver255;
            float	cx = h1 * oneOver255;
            float	cy = h2 * oneOver255;

            // find derivatives
            float	dx = (c - cx) * scale;
            float	dy = (c - cy) * scale;

            // normalize
			float	len = Math::fsqrt( dx*dx + dy*dy + 1 );

            // get normal
            float	nx = dx   / len;
            float	ny = -dy  / len;
            float	nz = 1.0f / len;

            // now convert to color and store in map
			byte * outPixel = nhm->getPixel(i, j);
            outPixel[0] = (byte)(128 + 127*nx);
            outPixel[1] = (byte)(128 + 127*ny);
            outPixel[2] = (byte)(128 + 127*nz);
            outPixel[3] = h;
        }
	}

	return nhm;
}

Math::vec3 Image::getNormal(int i, int j)
{
	byte	* pixelBytes	= getPixel(i, j);
	Math::vec3 normal;
	normal.x = float(pixelBytes[0]) / 127.0f - 1.0f;
	normal.y = float(pixelBytes[2]) / 127.0f - 1.0f;
	normal.z = float(pixelBytes[1]) / 127.0f - 1.0f;
	return normal;
}

Image * Image::buildNormalHeightMapFromNormals()
{
	Image * nhm = new Image(mWidth, mHeight, 1, Int8, RGBA);
	if(nhm == NULL) return NULL;
	nhm->setContentType(Vectors);

	const float oneOver255 = 1.0f / 255.0f;

	float minHeight = 0.0f;
	float maxHeight = 0.0f;

	const int MAX_NORMAL_MAP_SIZE = 2048;

	std::vector<std::vector<float> > heights;
	heights.resize(mWidth);

	uint32 i = 0, j = 0;

	for ( i = 0; i < mWidth; i++ )
	{
		heights[i].resize(mHeight);
		
		for ( j = 0; j < mHeight; j++ )
		{
			if(i == 0 && j == 0)
			{
				heights[i][j] = 0.0f;
				continue;
			}

			float height = 0.0f;
			Math::vec3 normal = getNormal(i, j);

			if(j == 0)//start of new line (lines are vertical)
			{
				//take start height from left column
				height = heights[i - 1][j];

				Math::vec3 leftNormal = getNormal(i - 1, j);

				float leftHeightChange = leftNormal.x - normal.x;

				height += leftHeightChange;
			}
			else
			{
				//take start height from upper row
				height = heights[i][j - 1];

				Math::vec3 upNormal = getNormal(i, j - 1);

				float upHeightChange = upNormal.z - normal.z;

				height += upHeightChange;
			}

			heights[i][j] = height;

			if(height < minHeight)
				minHeight = height;
			if(height > maxHeight)
				maxHeight = height;

			// copy normals
			byte * outPixel = nhm->getPixel(i, j);
			byte * inPixel = getPixel(i, j);
			outPixel[0] = inPixel[0];
			outPixel[1] = inPixel[1];
			outPixel[2] = inPixel[2];
		}
	}

	float heightAmpl = maxHeight - minHeight;
	
	for ( i = 0; i < mWidth; i++ )
	{
		for ( j = 0; j < mHeight; j++ )
		{
			float h = (heights[i][j] - minHeight) / heightAmpl;

			// now convert to color and store in map
			byte * outPixel = nhm->getPixel(i, j);
			outPixel[3] = byte(h * 255);
		}
	}

	return nhm;
}


Image * Image::downsample(uint32 level, int face)
{
	const Level& levelData = getLevel(level, face);

	//TODO: add depth and cube support
	ASSERT(levelData.dimensions.z == 1);

	if(levelData.dimensions.x <= sDownsampleLimit || levelData.dimensions.y <= sDownsampleLimit) return NULL;

	//now support only byte textures
	if(mDataType != Int8)
		return NULL;
	
	if(levelData.dimensions.x % 2 == 1 || levelData.dimensions.y % 2 == 1)
		return NULL;

	uint32 newW = levelData.dimensions.x >> 1;
	uint32 newH = levelData.dimensions.y >> 1;
	if (newW <1) newW = 1;
	if (newH <1) newH = 1;
	
    Image * img = new Image (newW, newH, 1, mDataType, mFormat);
	
	if(img == NULL)
		return NULL;

	float * acc = new float[ getComponentsNum() ];

 	uint32 i = 0, j = 0, n = 0, k = 0;
    for ( i = 0; i < levelData.dimensions.x; i += 2 )
	{
        for ( j = 0; j < levelData.dimensions.y; j += 2 )
        {
            uint32	iNext = (i+1) % levelData.dimensions.x;
            uint32	jNext = (j+1) % levelData.dimensions.y;

			byte * inPixel1 = getPixelFromLevel(level, i, j);
			byte * inPixel2 = getPixelFromLevel(level, i, jNext);
			byte * inPixel3 = getPixelFromLevel(level, iNext, j);
			byte * inPixel4 = getPixelFromLevel(level, iNext, jNext);
			byte * inPixels[4] = { inPixel1, inPixel2, inPixel3, inPixel4 };

			// sum up values
 			for ( n = 0; n < getComponentsNum(); ++n ) 
			{
				acc[n] = 0;
				for ( k = 0; k < 4; ++k )
				{
 					acc[n] += (float)inPixels[k][n];
				}
				acc[n] *= 0.25f;
            }

			byte * outPixel = img->getPixel(i >> 1, j >> 1);
			for ( n = 0; n < getComponentsNum(); ++n ) 
			{
				outPixel[n] = (byte)acc[n];
			}
        }
	}
	DELETE_ARR(acc);
    return img;

}

Image * Image::downsampleNHM(uint32 level, int face)
{
	const Level& levelData = getLevel(level, face);

	if(levelData.dimensions.x <= sDownsampleLimit || levelData.dimensions.y <= sDownsampleLimit) return NULL;
	ASSERT(levelData.dimensions.z == 1);

	uint32 newW = levelData.dimensions.x >> 1;
	uint32 newH = levelData.dimensions.y >> 1;
	if (newW <1) newW = 1;
	if (newH <1) newH= 1;
    Image * nhm = new Image (newW, newH, 1, Int8, mFormat);
	if(nhm == NULL) return NULL;

    const float	oneOver127 = 1.0f / 127.0f;
    //const float	oneOver255 = 1.0f / 255.0f;

    for ( uint32 i = 0; i < levelData.dimensions.x; i += 2 )
	{
        for ( uint32 j = 0; j < levelData.dimensions.y; j += 2 )
        {
            uint32	iNext = (i+1) % levelData.dimensions.x;
            uint32	jNext = (j+1) % levelData.dimensions.y;

			byte * inPixel1 = getPixelFromLevel(level, i, j);
			byte * inPixel2 = getPixelFromLevel(level, i, jNext);
			byte * inPixel3 = getPixelFromLevel(level, iNext, j);
			byte * inPixel4 = getPixelFromLevel(level, iNext, jNext);
			byte * inPixels[4] = { inPixel1, inPixel2, inPixel3, inPixel4 };

			Math::vec3 normal(0,0,0);
			float heightAcc = 0;
			// sum up values for RGB components, converting to [-1,1]
            for ( size_t k = 0; k < 4; ++k )
            {
				byte * inPixel = inPixels[ k ];
				for ( size_t n = 0; n < 3; ++n ) 
				{
					normal[n] += (oneOver127 * inPixel[n] - 1.0f);
				}
				if(getComponentsNum() > 3)
					heightAcc += (float)inPixel[3];
            }
			normal.normalize();
			heightAcc *= 0.25f;

			byte * outPixel = nhm->getPixel(i >> 1, j >> 1);

            // pack normalized vector into color values
            outPixel[0] = (byte)(128 + 127*normal[0]);
            outPixel[1] = (byte)(128 + 127*normal[1]);
            outPixel[2] = (byte)(128 + 127*normal[2]);
			if(getComponentsNum() > 3)
				outPixel[3] = (byte)(heightAcc);
        }
	}
    return nhm;
}

bool Image::buildMipMaps()
{
	mLevels	= 1;
	int face = 0;
	Image * next	= NULL;

	// downsample image
    while(true)
    {		
		if(getContentType() == Image::Vectors)
			next = downsampleNHM(mLevels - 1, face);
		else
			next = downsample(mLevels - 1, face);

		if ( next != NULL )
		{
			//get level data
			Level& data = next->mData[0];
			mData.push_back(data);

			++face;
			if(face == mFaces)
			{
				face = 0;
				++mLevels;
			}

			//disallow to delete image data when destructing [next]
			next->mData.erase(next->mData.begin());
			//destroy [next]
			DELETE_PTR(next);
		}
		else
		{
			return true;
		}
    }

	return true;
}

void Image::swapYZ()
{
	if(getComponentsNum() < 3)
		return;

	for(int level = 0; level < mLevels; ++level)
	{
		const Level& levelData = getLevel(level, 0);

		for ( uint32 i = 0; i < levelData.dimensions.x; i += 2 )
		{
			for ( uint32 j = 0; j < levelData.dimensions.y; j += 2 )
			{
				byte * pixel = getPixelFromLevel(level, i, j);
				byte tmp = pixel[1];
				pixel[1] = pixel[2];
				pixel[2] = tmp;
			}
		}
	}
}

bool Image::save(Data * data)
{
	uint32 i, j;

	for(i = 0; i < 4; ++i) data->putByte(GetNativeFileSign()[i]);

	IMG_FILE_HEADER header;
	header.size = sizeof(IMG_FILE_HEADER);
	header.version = IMG_FILE_VERSION;

	header.format		= static_cast<uint32>(mFormat);
	header.dataType		= static_cast<uint32>(mDataType);
	header.compression	= static_cast<uint32>(mCompression);
	header.depthType	= static_cast<uint32>(mDepthType);
	header.contentType	= static_cast<uint32>(mContentType);

	header.width		= static_cast<uint32>(mWidth);
	header.height		= static_cast<uint32>(mHeight);
	header.depth		= static_cast<uint32>(mDepth);
	header.levels		= static_cast<uint32>(mLevels);
	header.faces		= static_cast<uint32>(mFaces);

	data->putData(&header, header.size);

	for(i = 0; i < mLevels; ++i)
	{
		for(j = 0; j < mFaces; ++j)
		{
			const Level& level = getLevel(i, j);
			data->putData(level.data, level.size);
		}
	}

	return true;
}

bool Image::load(Data * data)
{
	uint32 i, j;

	char sign[4];
	for(i = 0; i < 4; ++i) 
	{
		sign[i] = data->readByte();
		//ASSERT(sign[i] == GetNativeFileSign()[i]);
	}

	IMG_FILE_HEADER header;
	data->readBytes(&header, sizeof(IMG_FILE_HEADER));

	ASSERT(header.size == sizeof(IMG_FILE_HEADER));
	ASSERT(header.version <= IMG_FILE_VERSION);

	mFormat			= static_cast<Format>(header.format);
	mDataType		= static_cast<DataType>(header.dataType);
	mCompression	= static_cast<Compression>(header.compression);
	mDepthType		= static_cast<DepthType>(header.depthType);
	mContentType	= static_cast<ContentType>(header.contentType);

	mWidth		= static_cast<uint32>(header.width);
	mHeight		= static_cast<uint32>(header.height);
	mDepth		= static_cast<uint32>(header.depth);
	mLevels		= static_cast<uint32>(header.levels);
	mFaces		= static_cast<uint32>(header.faces);

    uint32			w		= getWidth();
    uint32			h		= getHeight();
	uint32			d		= getDepth();

	for(i = 0; i < mLevels; ++i)
	{
		size_t dataSize = calcCompressedDataSize(w, h, d);
		Level level;

		for(j = 0; j < mFaces; ++j)
		{
			//create data 
			level.dimensions = tuple3i(w, h, d);
			level.size		= dataSize;
			level.data		= new byte[dataSize];
			ASSERT( level.data );
			mData.push_back(level);

			//read data
			data->readBytes(level.data, dataSize);
		}

		w = ( w > 1) ? w >> 1 : 1;
		h = ( h > 1) ? h >> 1 : 1;
	    d = ( d > 1) ? d >> 1 : 1;
	}

	mNative = true;

	return true;
}


}//namespace RenderData { 

}//namespace Squirrel {

