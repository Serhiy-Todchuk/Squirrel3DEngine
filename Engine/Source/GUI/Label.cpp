// Label.cpp: implementation of the Label class.
//
//////////////////////////////////////////////////////////////////////

#include "Label.h"
#include "Render.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Label, GUILabel);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
 
Label::Label()
{
	SQREFL_SET_CLASS(GUI::Label);
	setSizeSep(32,16);
}

Label::~Label()
{

}

void Label::draw()
{
	tuple2i p=getGlobalPos();

	if(getTextAlignment() == Element::textCenter)
	{
		int width = getSize().x;

		GUI::Font * font = Render::Instance().getFont(getFontSize());

		int strWidth = font->getStrWidth(getText().c_str());
		p.x += Math::maxValue( ((width - strWidth) / 2), 0);
	}

	//draw text
	Render::Instance().drawText(p, getText(), getFontSize());
}

Element * Label::recieveEvent(EventType e, int value)
{
	return Element::recieveEvent(e, value);//TODO: NULL
}

}//namespace GUI { 
}//namespace Squirrel {