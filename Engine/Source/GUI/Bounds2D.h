// Element.h: interface for the Element class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <common/tuple.h>

namespace Squirrel {
namespace GUI { 

class Bounds2D
{
public:
	tuple2i mMin;
	tuple2i mMax;

	Bounds2D(): mMin(0, 0), mMax(0, 0) {}
	Bounds2D(const tuple2i& min_, const tuple2i& max_): mMin(min_), mMax(max_) {}
	Bounds2D(const tuple2i& pos, int sizeX, int sizeY): mMin(pos), mMax(pos + tuple2i(sizeX, sizeY))	{}
	tuple2i getSize() const { return mMax - mMin; }

	bool intersects(const Bounds2D& otherBounds)
	{
		int ret = 0;
		tuple2i v1[2];
		tuple2i v2[2];
		v1[1] = mMin;
		v1[0] = mMax;
		v2[1] = otherBounds.mMin;
		v2[0] = otherBounds.mMax;
		if( (v1[0].x<=v2[0].x)	&	(v1[1].x>=v2[1].x)	&
			(v1[0].y<=v2[0].y)	&	(v1[1].y>=v2[1].y)	)
			return true;//[this] polnostyu vhodit v [otherBounds];
		if( (v2[0].x<=v1[0].x)	&	(v2[1].x>=v1[1].x)	&
			(v2[0].y<=v1[0].y)	&	(v2[1].y>=v1[1].y)	)
			return true;//[otherBounds] polnostyu vhodit v [this];
		if(	(	(v1[0].x<v2[0].x)	&	(v1[0].x>=v2[1].x)	)	|
			(	(v2[0].x<v1[0].x)	&	(v2[0].x>=v1[1].x)	)	|
			(	(v1[1].x<v2[0].x)	&	(v1[1].x>=v2[1].x)	)	|		
			(	(v2[1].x<v1[0].x)	&	(v2[1].x>=v1[1].x)	)	)		
			ret++;
		if(	(	(v1[0].y<v2[0].y)	&	(v1[0].y>=v2[1].y)	)	|
			(	(v2[0].y<v1[0].y)	&	(v2[0].y>=v1[1].y)	)	|
			(	(v1[1].y<v2[0].y)	&	(v1[1].y>=v2[1].y)	)	|		
			(	(v2[1].y<v1[0].y)	&	(v2[1].y>=v1[1].y)	)	)		
			ret++;
		if(ret==2) return true;//[otherBounds] peresekaetsya s [this];
		return false;
	}

	bool contains(const tuple2i& point)
	{
		if((point.x<=mMax.x)&&(point.x>=mMin.x)&&(point.y<=mMax.y)&&(point.y>=mMin.y))
			return true;
		return false;
	}
	
};

}//namespace GUI { 
}//namespace Squirrel {