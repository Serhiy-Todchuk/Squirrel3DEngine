#include "Slider.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Slider, GUISlider);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Slider::Slider(): mPoint(new MovingPoint)
{
	SQREFL_SET_CLASS(GUI::Slider);
	wrapAtomicField("Value", &mValue);

	mValue = 0.0f;
	mVertical = false;

	mPoint->setMaster(this);
	mPoint->setSizeSep(8,8);
}

Slider::~Slider()
{
}

void	Slider::setValue(float val)
{
	mValue = Math::clamp<float>(val,0,1);
	float len   = (float) (mVertical ? getSize().y : getSize().x);
	float ptLen = (float) (mVertical ? mPoint->getSize().y : mPoint->getSize().x);
	float ptPos = (len-ptLen) * mValue;
	mPoint->setPosSep( mVertical ? mPoint->getPos().x : (int)ptPos, mVertical ? (int)ptPos : mPoint->getPos().y );
	processAction(VALUE_CHANGED_ACTION, this);
}

void	Slider::setVertical(bool vert)
{
	if(mVertical!=vert)
	{
		setSize(getSize());
		setValue(mValue);
	}
	mVertical = vert;
}

Element * Slider::onLU()
{

	return this;
}

Element * Slider::onLD()
{
	if(!mPoint->isIn(Cursor::Instance().getPos()))
		movePointByMouse();
	return mPoint->onLD();
}

void Slider::setPos(tuple2i pos)
{
	Element::setPos(pos);
	mPoint->setMasterPos( getGlobalPos() );
}

Element * Slider::onMW(float delta)
{
	tuple2i cursor = Cursor::Instance().getPos();
	if(isIn(cursor))
	{
		setValue(mValue + delta/40);
		return this;
	}
	return NULL;
}

void Slider::draw()
{
	if(getDrawStyle() > 0)
	{
		tuple2i p = getGlobalPos();
		Render::Instance().drawBackPlane(p, getSize());
		Render::Instance().drawBorder(p, getSize(), getState() == Element::stateActive);
	}
	mPoint->draw();
}

void Slider::resetState()
{
	Element::resetState();
	mPoint->resetState();
}

void Slider::movePointByMouse()
{
	int cursor		= mVertical	?
						(Cursor::Instance().getPos().y - getGlobalPos().y)	:
						(Cursor::Instance().getPos().x - getGlobalPos().x);

	float ptLen = (float) (mVertical ? mPoint->getSize().y:	mPoint->getSize().x);
	float ptPos = cursor - (ptLen/2);
	mPoint->setPosSep( mVertical ? mPoint->getPos().x : (int)ptPos, mVertical ? (int)ptPos : mPoint->getPos().y );
	ptMoved();
}

Element * Slider::recieveEvent(EventType e, int mValue)
{
	Element * elem = mPoint->Element::recieveEvent(e,mValue);
	if(elem) return elem;
	return Element::recieveEvent(e,mValue);
}

//ActionDelegate definition
bool Slider::processAction(const std::string& action, Element * sender) 
{
	if(sender == this)
	{
		return Element::processAction(action, sender);
	}
	else if(sender == mPoint.get())
	{
		ptMoved();
	}
	return true;
}

void Slider::ptMoved()
{
	mPoint->setPosSep(  
		Math::clamp<int>( mPoint->getPos().x, 0, getSize().x-mPoint->getSize().x ),
		Math::clamp<int>( mPoint->getPos().y, 0, getSize().y-mPoint->getSize().y )
				);

	float len   = (float) (mVertical ? getSize().y		:	getSize().x);
	float ptLen = (float) (mVertical ? mPoint->getSize().y:	mPoint->getSize().x);
	float ptPos = (float) (mVertical ? mPoint->getPos().y	:	mPoint->getPos().x);
	mValue = ptPos/(len-ptLen);
	processAction(VALUE_CHANGED_ACTION, this);
}

void Slider::setSize(tuple2i size) 
{ 
	Element::setSize(size); 
	mPoint->setSizeSep(mVertical ? size.x : mPoint->getSize().x, mVertical ? mPoint->getSize().y : size.y);
}

void Slider::setPointSize(int size_)
{
	mPoint->setSizeSep(mVertical ? mPoint->getSize().x : size_, mVertical ? size_ : mPoint->getSize().y);
}

void Slider::setPointSize(float size_)
{
	int len = mVertical ? getSize().y : getSize().x;
	int s   = int(size_ * len);
	mPoint->setSizeSep(mVertical ? mPoint->getSize().x : s, mVertical ? s : mPoint->getSize().y);
}

}//namespace GUI { 
}//namespace Squirrel {