#pragma once

#include "macros.h"

namespace Squirrel {

struct SQCOMMON_API PixelFormat
{
	PixelFormat(): 
		colorBits(32), alphaBits(8),
		depthBits(16), stencilBits(8),
		doubleBuffer(true), samples(0),
		accumulation(0) {}

	int		colorBits;
	int		alphaBits;
	int		depthBits;
	int		stencilBits;
	bool	doubleBuffer;
	int		samples;
	int		accumulation;
};

struct SQCOMMON_API Caps
{
	bool	doubleBuffer;
	int		multiSampleLevel;
	int		stencilBits;
	int		colorBits;
	int		alphaBits;
	int		depthBits;
	int		accumBits;
	float	maxAnisotropy;
	int		maxTextureUnits;
	int		max2DTextureSize;
	int		max3DTextureSize;
	int		maxCubeTextureSize;
	
	int		videoMemoryMegabytes;
	int		textureMemoryMegabytes;
};

class SQCOMMON_API Context
{
protected:
	Caps mCaps;
	PixelFormat mPixelFormat;

private:
	bool mCapsInitialized;

protected:
	Context(const PixelFormat& pf): 
		mPixelFormat(pf), 
		mCapsInitialized(false) 
	{}

	virtual bool initCaps() = 0;

public:
	virtual ~Context()	{}

	const Caps&			getCaps();
	const PixelFormat&	getPixelFormat()	const	{ return mPixelFormat; }
	bool getCapsInitialized() const { return mCapsInitialized; }

	virtual void		activate()						= 0;
	virtual void		deactivate()					= 0;
	virtual Context *	createSibling()	const			= 0;
	virtual void		setSwapInterval(int interval)	= 0;
	virtual void		finishFrame()					= 0;

	virtual bool		isOk() const { return true; }
};

}//namespace Squirrel {