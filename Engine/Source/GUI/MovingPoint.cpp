#include "MovingPoint.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

const char_t * MovingPoint::MOVING_ACTION = "pointMoved";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MovingPoint::MovingPoint()
{
	SQREFL_SET_CLASS(GUI::MovingPoint);
}

MovingPoint::~MovingPoint()
{

}

Element * MovingPoint::onLU()
{
	setState(Element::stateStd);
	return this;
}

Element * MovingPoint::onLD()
{
	setState(Element::stateActive);
	mPrevPos = Cursor::Instance().getPos() - getPos();
	return this;
}

Element * MovingPoint::onMM()
{
	if(getState() == stateActive) 
	{
		tuple2i newPos = Cursor::Instance().getPos() - mPrevPos;
		setPos(newPos);
		
		if(getMaster() != NULL)
		{
			getMaster()->processAction(MOVING_ACTION, this);
		}
		return this;
	}
	return NULL;
}

}//namespace GUI { 
}//namespace Squirrel {