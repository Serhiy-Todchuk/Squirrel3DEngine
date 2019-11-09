
#pragma once

#include "MovingPoint.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Slider  : public Element
{
private:
	void movePointByMouse();
	void ptMoved();

	std::auto_ptr<MovingPoint> mPoint;

	float mValue;
	bool mVertical;

public:
	Slider();
	virtual ~Slider();

	inline MovingPoint *	getPoint()		{ return mPoint.get(); }
	inline float			getValue()		{ return mValue; }
	inline bool				isVertical()	{return mVertical;}

	void	setValue(float val);
	void	setSize(tuple2i size);
	void	setPointSize(int size);
	void	setPointSize(float size);
	void	setVertical(bool vert);

	virtual Element * onLU();
	virtual Element * onLD();
	virtual Element * onMW(float delta);

	virtual Element * recieveEvent(EventType e, int value);

	virtual bool processAction(const std::string& action, Element * sender);

	virtual void resetState();
	virtual void setPos(tuple2i pos);
	virtual void draw();
};

}//namespace GUI { 
}//namespace Squirrel {