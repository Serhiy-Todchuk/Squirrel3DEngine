// Button.cpp: implementation of the Button class.
//
//////////////////////////////////////////////////////////////////////

#include "Button.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Button, GUIButton);

const char_t * Button::LEFT_CLICK_ACTION = "leftClick";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Button::Button()
{
	SQREFL_SET_CLASS(GUI::Button);
	setSizeSep(40,16);
}

Button::~Button()
{

}

void Button::draw()
{
	if(getDrawStyle() == 0) return;

	tuple2i p=getGlobalPos();

	bool pressed = getState() == Element::stateActive;
	Render::Instance().drawPlane(p, getSize(), pressed);
	Render::Instance().drawBorder(p, getSize(), pressed);
	Render::Instance().drawText(p, getText(), getFontSize());
}

Element * Button::onLU()
{
	if(getState() == Element::stateActive)
	{
		setState(Element::stateStd);
		processAction(std::string(LEFT_CLICK_ACTION),this);
		return this;
	}
	return NULL;
}

Element * Button::onMM()
{
	tuple2i cursor = Cursor::Instance().getPos();
	if(isIn(cursor) && getState() != Element::stateActive)
	{
		setState(Element::stateOver);
	}
	return NULL;
}

Element * Button::onRU()
{
	
	return NULL;
}

Element * Button::onLD()
{
	setState(Element::stateActive);
	return this;
}

Element * Button::onRD()
{

	return NULL;
}


}//namespace GUI { 
}//namespace Squirrel {