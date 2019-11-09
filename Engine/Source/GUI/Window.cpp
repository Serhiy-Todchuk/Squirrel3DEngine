#include "Window.h"
#include "Render.h"
#include "Cursor.h"
#include "Manager.h"
#include "Bounds2D.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Window, GUIWindow);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Window::Window()
{
	SQREFL_SET_CLASS(GUI::Window);

	wrapAtomicField("HeaderHeight", &mHeaderHeight);

	mHeaderHeight = 16;
	mMargin.y = mHeaderHeight;
}

Window::~Window()
{

}

void Window::draw()
{
	Element::draw();

	GUI::Font * font = Render::Instance().getFont(getFontSize());

	tuple2i p=getGlobalPos();
	int wndWidth = getSize().x;

	Render::Instance().drawRect(p + tuple2i(1, 1), tuple2i(wndWidth - 2, mHeaderHeight - 1), Math::vec4(0.5f, 0.7f, 0.4f, 0.6f)); 
	Render::Instance().drawBorder(p, tuple2i(wndWidth, mHeaderHeight), getState() == Element::stateActive);

	int strWidth = font->getStrWidth(getText().c_str());
	p.x += (wndWidth - strWidth) / 2;

	Render::Instance().drawText(p, getText(), getFontSize());

	drawContent();
	Render::Instance().flush();
}

Element * Window::onLD()
{
	tuple2i cursor = Cursor::Instance().getPos();
	tuple2i pos = getGlobalPos();
	tuple2i size(getSize().x, mHeaderHeight);
	if(Bounds2D(pos, pos + size).contains(cursor))
	{
		setState( Element::stateActive );
	}
	return this;
}

Element * Window::onMM()
{
	if(getState()==0 && getDragable()!=0)
	{
		tuple2i newPos = getPos() + Cursor::Instance().getPosDiff();
		setPos(newPos);
	}
	return this;
}

}//namespace GUI { 
}//namespace Squirrel {