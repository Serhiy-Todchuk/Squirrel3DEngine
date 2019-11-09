#pragma once

#include <common/tuple.h>
#include <string>
#include "macros.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Cursor
{
public:

	enum AppearanceType
	{
		tNormal = 0,
		tWait,
		tText,
		tMove,
		tSizeNWSE,
		tSizeNESW,
		tSizeNS,
		tSizeWE,
		tHelp,
		tStop,
		tStarting,
		tLink,
		tNUMBEROFTYPES
	};

private:
	Cursor();

	int mState;
	tuple2i mPos;
	tuple2i mPrevPos;

	bool mScroll;

	AppearanceType mAppearance;

public:


	static Cursor& Instance();

	virtual ~Cursor();

	AppearanceType getAppearance() { return mAppearance; }
	void setAppearance(AppearanceType appearance);

	void setScroll(bool s)	{ mScroll = s; }
	bool getScroll() const	{ return mScroll; }

	void setState(int s)	{ mState = s; }
	int getState()	const	{ return mState; }

	const tuple2i& getPos()		const	{ return mPos; }
	const tuple2i& getPrevPos()	const	{ return mPrevPos; }
	tuple2i getPosDiff()		const	{ return mPos - mPrevPos; }

	void setPos(tuple2i p);
	void changePos(tuple2i p);
	void destroy();

	void draw();
};

}//namespace GUI { 
}//namespace Squirrel {