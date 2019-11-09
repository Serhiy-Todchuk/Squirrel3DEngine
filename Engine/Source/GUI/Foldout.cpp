// Foldout.cpp: implementation of the Foldout class.
//
//////////////////////////////////////////////////////////////////////

#include "Foldout.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Foldout, GUIFoldout);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Foldout::Foldout()
{
	SQREFL_SET_CLASS(GUI::Foldout);
	wrapAtomicField("Show Fold", &mShowFold);

	setSizeSep(40,16);

	mShowFold = true;
	mIntendance = 0;

	mIntendanceSize = 0;
}

Foldout::~Foldout()
{

}

void Foldout::draw()
{
	if(getDrawStyle() == 0) return;

	Font * font = Render::Instance().getFont(getFontSize());

	const float foldSize = 0.62f;

	tuple2i p = getGlobalPos();
	float textSize = static_cast<float>(font->getCharHeight());
	int oneSize = static_cast<int>(textSize * foldSize);
	tuple2i size(oneSize, oneSize);
	const int foldOffset = static_cast<int>((textSize - oneSize + 1)/2);

	mIntendanceSize = static_cast<float>(oneSize + foldOffset * 2);
	p.x += mIntendanceSize * mIntendance;

	if(mShowFold)
	{
		bool expanded = getState() == Element::stateActive;
		Render::Instance().drawFoldout(p + tuple2i(foldOffset, foldOffset), size, expanded);
	}
	p.x += mIntendanceSize - Render::TEXT_OFFSET.x;
	Render::Instance().drawText(p, getText(), getFontSize());
	//update width
	//int width = p.x + Render::TEXT_OFFSET.x + font->getStrWidth(getText().c_str()) + 2;
	//setSizeSep(width, getSize().y);
}

Element * Foldout::onLU()
{

	return this;
}

Element * Foldout::onLD()
{
	if(!mShowFold) return this;

	Font * font = Render::Instance().getFont(getFontSize());
	int textSize = font->getCharHeight();
	tuple2i p = getGlobalPos();
	p.x += mIntendance * mIntendanceSize;
	tuple2i s(textSize, textSize);
	tuple2i mouse = Cursor::Instance().getPos();
	if(IsIn(p, p + s, mouse) )
	{
		//toggle state
		setState( getState() !=	Element::stateActive ? 
									Element::stateActive : 
									Element::stateStd );
	}
	return this;
}

void Foldout::resetState()
{
	//empty
}

}//namespace GUI { 
}//namespace Squirrel {