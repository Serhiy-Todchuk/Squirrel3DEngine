// Sizer.h: interface for the Sizer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 


class SQGUI_API Sizer : public Element
{
protected:
	enum SizingDirection
	{
		north = 1,
		northEast,
		east,
		southEast,
		south,
		southWest,
		west,
		northWest
	} direction;

	tuple2i mMinSize;
	tuple2i mClickedPos;
	tuple2i mClickedSize;

public:

	Sizer();
	virtual ~Sizer();

	virtual Element * onLU();
	virtual Element * onLD();
	virtual Element * onMM();

	virtual void draw();

	void		setMinSize(tuple2i minSize)	{ mMinSize = minSize; }
	tuple2i		getMinSize()				{ return mMinSize; }

};

}//namespace GUI { 
}//namespace Squirrel {