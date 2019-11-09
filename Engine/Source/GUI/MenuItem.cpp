// MenuItem.cpp: implementation of the MenuItem class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuItem.h"
#include "Label.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::MenuItem, GUIMenuItem);

using namespace Reflection;

const char_t * MenuItem::EXPOSE_ACTION	= "expose";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MenuItem::MenuItem()
{
	SQREFL_SET_CLASS(GUI::MenuItem);

	mSubMenu = false;
}

MenuItem::~MenuItem()
{

}

Element * MenuItem::onLU()
{
	processAction(EXPOSE_ACTION, this);
	return this;
}

Element * MenuItem::onRU()
{
	processAction(EXPOSE_ACTION, this);
	return this;
}

void MenuItem::draw()
{
	if(getDrawStyle() == 0) return;

	Font * font = Render::Instance().getFont(getFontSize());
	float textHeight = static_cast<float>(font->getCharHeight());

	const float foldSizeCoef = 0.618f;

	tuple2i pos = getGlobalPos();
	tuple2i p = pos;

	//draw backplane

	if(getState() != Element::stateStd)
	{
		bool pressed = getState() == Element::stateActive;
		Render::Instance().drawPlane(p, getSize(), pressed);
	}

	//TODO: draw icon here

	//draw caption

	float iconColumnWidth = getSize().y;

	p.x += iconColumnWidth + Render::TEXT_OFFSET.x;

	Render::Instance().drawText(p, getText(), getFontSize());

	//draw hotkey

	if(mHotkey.length() > 0)
	{
		//TODO: draw hotkey here
	}

	//draw fold

	if(mSubMenu)
	{
		int foldSize = static_cast<int>(textHeight * foldSizeCoef);
		const int foldOffset = static_cast<int>((textHeight - foldSize + 1)/2);

		float foldColumnWidth = getSize().y;
		p.x = pos.x + getSize().x - foldColumnWidth;

		Render::Instance().drawFoldout(p + tuple2i(foldOffset, foldOffset), tuple2i(foldSize, foldSize), false);
	}
}

}//namespace GUI { 
}//namespace Squirrel {