// Element.cpp: implementation of the Element class.
//
//////////////////////////////////////////////////////////////////////

#include "Element.h"
#include "Cursor.h"
#include "Render.h"
#include <common/Log.h>
#include <Reflection/EnumWrapper.h>

namespace Squirrel {
namespace GUI { 

ElemFactory& ElemFactory::Instance()
{
	static ElemFactory instance;
	return instance;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Element *			Element::sActive = 0;
const char_t *		Element::SIZE_CHANGED_ACTION		= "sizeChanged";
const char_t *		Element::POSITION_CHANGED_ACTION	= "positionChanged";
const char_t *		Element::STATE_CHANGED_ACTION		= "stateChanged";
const char_t *		Element::VALUE_CHANGED_ACTION		= "valueChanged";

Element::Element()
{
	mFontSize	= Render::sizeNormal;
	mTextAlignment = textLeft;
	mMaster		= 0;
	mState		= stateStd;
	mVisible	= true;
	mDragable	= true;
	mDrawStyle	= 1;
	mMasterPos	= tuple2i(0, 0);
	mPos = tuple2i(0,0);
	mSize = tuple2i(8,8);
	mText = "";
	mHelp = "";
	mActionDelegate = 0;
	mId = -1;

	mVerticalSizing		= bindStrictly;
	mHorizontalSizing	= bindStrictly;

	mBindingTop			= bindFlexible;
	mBindingBottom		= bindStrictly;
	mBindingLeft		= bindFlexible;
	mBindingRight		= bindStrictly;

	mActionMap.clear();

	SQREFL_SET_CLASS(GUI::Element)

	wrapAtomicField( "name", &mName );
	wrapAtomicField( "pos", &mPos.x, 2 );
	wrapAtomicField( "size", &mSize.x, 2 );
	wrapAtomicField( "text", &mText );
	wrapAtomicField( "help", &mHelp );
	wrapAtomicField( "visible", &mVisible );

	Reflection::EnumWrapper * enumWrapper = new Reflection::EnumWrapper(reinterpret_cast<int32 *>(&mFontSize));
	enumWrapper->addStringForValue(Render::sizeSmall,	"small size");
	enumWrapper->addStringForValue(Render::sizeNormal,	"normal size");
	enumWrapper->addStringForValue(Render::sizeBig,		"big size");
	addField("fontSize", enumWrapper);

	enumWrapper = new Reflection::EnumWrapper(reinterpret_cast<int32 *>(&mTextAlignment));
	enumWrapper->addStringForValue(textLeft,	"left");
	enumWrapper->addStringForValue(textCenter,	"center");
	enumWrapper->addStringForValue(textRight,	"right");
	addField("textAlignment", enumWrapper);
}

Element::~Element()
{
	if(sActive == this) {
		ResetFocus();
	}
}

bool Element::isIn(tuple2i pt)
{
	tuple2i pos = getGlobalPos();
	return Bounds2D(pos, pos + getSize()).contains(pt);
}

int Element::isIntersect(tuple2i pos, tuple2i size)
{
	Bounds2D otherBounds(pos, pos + size);

	tuple2i thisPos		= getGlobalPos();
	tuple2i thisSize	= getSize();
	Bounds2D thisBounds(thisPos, thisPos + thisSize);

	return thisBounds.intersects(otherBounds) ? 1 : 0;
}

tuple2i Element::getGlobalPos()
{
	return mPos + mMasterPos;
}

void Element::clipContent()
{
	//setup new constraints
	tuple2i p = getGlobalPos();
	mStoredClipLines = Render::Instance().getClipLines();
	Math::vec4 newClipLines = mStoredClipLines;
	tuple2i s = p + getSize();
	if(newClipLines.x < p.y) newClipLines.x = (float)p.y;//up
	if(newClipLines.y > s.y) newClipLines.y = (float)s.y;//down
	if(newClipLines.z < p.x) newClipLines.z = (float)p.x;//left
	if(newClipLines.w > s.x) newClipLines.w = (float)s.x;//right
	Render::Instance().enableClipLines( newClipLines );
}

void Element::unclipContent()
{
	//return parent's constraints
	Render::Instance().enableClipLines( mStoredClipLines );
}

void Element::draw()
{
	if(getDrawStyle() > 0)
	{
		tuple2i p = getGlobalPos();
		Render::Instance().drawPlane(p, getSize(), getState() == Element::stateActive);
		Render::Instance().drawBorder(p, getSize(), getState() == Element::stateActive);
	}
}

bool Element::doesAcceptEvent(EventType e)
{
	tuple2i cursor = Cursor::Instance().getPos();
	switch(e)
	{
	case leftDouble:	
	case leftUp:		
	case leftDown:		
	case rightUp:		
	case rightDown:		
	case mouseWheel:	
	case checkUnderMouse:
		return isIn(cursor);	
	case keyChar:		
	case keyDown:		
	case keyUp:			
		return (this == sActive);
	case mouseMove:		
		
		if(isIn(cursor))
			return true;
		else
			return (this == sActive);
	}
	return false;
}

Element * Element::recieveEvent(EventType e, int value)
{
	if(!doesAcceptEvent(e))
		return NULL;

	switch(e)
	{
	case leftDouble:	
		return onDBL();			
	case leftUp:		
		return onLU();				
	case leftDown:		
		return onLD();				
	case rightUp:		
		return onRU();				
	case rightDown:		
		return onRD();				
	case keyChar:		
		return onCh(value);		
	case keyDown:		
		return onKD(value);		
	case keyUp:			
		return this;				
	case mouseWheel:	
		return onMW((float)value);	
	case mouseMove:		
		return onMM();				
	case checkUnderMouse:
		return this;
	}

	return NULL;
}

bool Element::IsIn(tuple2i min_, tuple2i max_, tuple2i pt)
{
	if((pt.x<=max_.x)&(pt.x>=min_.x)&(pt.y<=max_.y)&(pt.y>=min_.y))
		return true;
	return false;
}

void Element::setFocus()
{
	if(sActive)
	{
		if(sActive == this)
		{
			return;
		}
		sActive->onUnFocus();
	}
	sActive = this;
	this->onFocus();
}

void Element::ResetFocus()
{
	if(sActive) sActive->onUnFocus();
	sActive = NULL;
}

Element * Element::GetFocus()
{
	return sActive;
}

void Element::SetFocus(Element * elem)
{
	if(elem)
		elem->setFocus();
	else
		ResetFocus();
}

void Element::addAction(const std::string& action, ActionHandler * handler)
{
	if(!action.length() || !handler) return;
	ActionHandlersMap::iterator it = mActionMap.find(action);
	if(it != mActionMap.end() && it->second)
	{
		if(it->second == handler)
			return;
		DELETE_PTR(it->second);
	}
	mActionMap[action] = handler;
}

bool Element::executeAction(const std::string& action)
{
	if(!action.length()) return false;
	ActionHandlersMap::iterator it = mActionMap.find(action);
	if(it==mActionMap.end() || !it->second)
		return false;
	it->second->execute(action, this);
	return true;
}

void Element::clearActions()
{
	for(ActionHandlersMap::iterator it = mActionMap.begin();
		it != mActionMap.end(); ++it)
	{
		DELETE_PTR(it->second);
	}
	mActionMap.clear();
}

//ActionDelegate definition
bool Element::processAction(const std::string& action, Element * sender) 
{
	bool ret = false;
	if(mActionDelegate)
	{
		ret = mActionDelegate->processAction(action,sender);
		if(ret) 
			return true; 
	}
	ret = executeAction(action);
	return ret;
}

void Element::updateAutoLayout(tuple2i parentSize)
{
	//if(!mAutoLayout) return; << //TODO:

	if(mMaster == NULL) return;

	//skip default configuration
	if(	mVerticalSizing		== bindStrictly &&
		mHorizontalSizing	== bindStrictly &&
		mBindingTop			== bindFlexible &&
		mBindingBottom		== bindStrictly &&
		mBindingLeft		== bindFlexible &&
		mBindingRight		== bindStrictly)
		return;

	//calc initial margins
		
	float bottomMargin		= mInitialBounds.mMin.y - mInitialParentBounds.mMin.y;
	float topMargin			= mInitialParentBounds.mMax.y - mInitialBounds.mMax.y;
	float leftMargin		= mInitialBounds.mMin.x - mInitialParentBounds.mMin.x;
	float rightMargin		= mInitialParentBounds.mMax.x - mInitialBounds.mMax.x;
	
	//get current bounds
	
	//skip position of parent as we work in parent space
	Bounds2D parentBounds(tuple2i(0,0), parentSize);

	Bounds2D bounds				= getBounds();
		
	//calc current margins
	
	float newBottomMargin	= bounds.mMin.y - parentBounds.mMin.y;
	float newTopMargin		= parentBounds.mMax.y - bounds.mMax.y;
	float newLeftMargin		= bounds.mMin.x - parentBounds.mMin.x;
	float newRightMargin	= parentBounds.mMax.x - bounds.mMax.x;
	
	//correct margins to parent's edges
	
	//strict bindings
	
	if(mBindingLeft == BindingType::bindStrictly)
		newLeftMargin = leftMargin;
	
	if(mBindingRight == BindingType::bindStrictly)
		newRightMargin = rightMargin;		
	
	if(mBindingTop == BindingType::bindStrictly)
		newTopMargin = topMargin;
	
	if(mBindingBottom == BindingType::bindStrictly)
		newBottomMargin = bottomMargin;
	
	//interpolate flexible bindings
	
	//horizontal axis
	
	float xScaleFactor = parentBounds.getSize().x / mInitialParentBounds.getSize().x;	
			
	float xLeftFactor = bounds.mMax.x - parentBounds.mMin.x;
	float xLeftInitialFactor = mInitialBounds.mMax.x - mInitialParentBounds.mMin.x;
	float xLeftScaleFactor = xLeftFactor / xLeftInitialFactor;
	
	float xRightFactor = parentBounds.mMax.x - bounds.mMin.x;
	float xRightInitialFactor = mInitialParentBounds.mMax.x - mInitialBounds.mMin.x;
	float xRightScaleFactor = xRightFactor / xRightInitialFactor;
	
	if(mHorizontalSizing == BindingType::bindFlexible)
	{
		if(mBindingLeft == BindingType::bindFlexible && mBindingRight == BindingType::bindFlexible)
		{
			//whole horizontal axis is flexible
			
			newLeftMargin = leftMargin * xScaleFactor;			
			newRightMargin = rightMargin * xScaleFactor;				
		}
		else if(mBindingLeft == BindingType::bindFlexible)
		{
			//left part of horizontal axis is flexible
		
			newLeftMargin = leftMargin * xLeftScaleFactor;				
		}
		else if(mBindingRight == BindingType::bindFlexible)
		{
			//right part of horizontal axis is flexible
			
			newRightMargin = rightMargin * xRightScaleFactor;
		}
	}
	
	//vertical axis
	
	float yScaleFactor = parentBounds.getSize().y / mInitialParentBounds.getSize().y;
			
	float yBottomFactor = bounds.mMax.y - parentBounds.mMin.y;
	float yBottomInitialFactor = mInitialBounds.mMax.y - mInitialParentBounds.mMin.y;
	float yBottomScaleFactor = yBottomFactor / yBottomInitialFactor;
		
	float yTopFactor = parentBounds.mMax.y - bounds.mMin.y;
	float yTopInitialFactor = mInitialParentBounds.mMax.y - mInitialBounds.mMin.y;
	float yTopScaleFactor = yTopFactor / yTopInitialFactor;		
		
	if(mVerticalSizing == BindingType::bindFlexible)
	{
		if(mBindingBottom == BindingType::bindFlexible && mBindingTop == BindingType::bindFlexible)
		{
			//whole vertical axis is flexible
			
			newBottomMargin = bottomMargin * yScaleFactor;			
			newTopMargin = topMargin * yScaleFactor;				
		}
		else if(mBindingBottom == BindingType::bindFlexible)
		{
			//bottom part of vertical axis is flexible
			
			newBottomMargin = bottomMargin * yBottomScaleFactor;				
		}
		else if(mBindingTop == BindingType::bindFlexible)
		{
			//top part of vertical axis is flexible
			
			newTopMargin = topMargin * yTopScaleFactor;
		}
	}
	else
	{
		if(mBindingBottom == BindingType::bindFlexible && mBindingTop == BindingType::bindFlexible)
		{
			//case when control should not be scaled and centered!
			
		}
	}
	
	//build new bounds from margins
	
	tuple2i newMin = tuple2i(parentBounds.mMin.x + newLeftMargin, 
	                             parentBounds.mMin.y + newBottomMargin);
	tuple2i newMax = tuple2i(parentBounds.mMax.x - newRightMargin, 
	                             parentBounds.mMax.y - newTopMargin);
	Bounds2D newBounds		= Bounds2D(newMin, newMax);
	
	//correct size of control
	
	tuple2i newSize = newBounds.getSize();
	tuple2i initialSize = mInitialBounds.getSize();
	
	if(mHorizontalSizing == BindingType::bindStrictly)
	{
		newSize.x = initialSize.x;
	}
	
	if(mVerticalSizing == BindingType::bindStrictly)
	{
		newSize.y = initialSize.y;
	}
	
	/*
	//keep aspect ratio
	if(_keepAspectRatio)
	{
		float initialAspect	= initialSize.x / initialSize.y;
		float newAspect		= newSize.x / newSize.y;
		if(initialAspect != newAspect)
		{
			if(initialAspect > newAspect)
			{
				newSize.y = newSize.x / initialAspect;
			}
			else
			{
				newSize.x = newSize.y * initialAspect;					
			}
		}
	}
	*/
	
	/*
	//clamp size
	if(_minSize.x > 0 && newSize.x < _minSize.x)
	{
		newSize.x = _minSize.x;
	}
	if(_minSize.y > 0 && newSize.y < _minSize.y)
	{
		newSize.y = _minSize.y;
	}
	if(_maxSize.x > 0 && newSize.x > _maxSize.x)
	{
		newSize.x = _maxSize.x;
	}
	if(_maxSize.y > 0 && newSize.y > _maxSize.y)
	{
		newSize.y = _maxSize.y;
	}
	*/

	//assign new size
	
	if(mBindingLeft == BindingType::bindStrictly)
	{
		newBounds.mMax.x = newBounds.mMin.x + newSize.x;
	}
	else if(mBindingRight == BindingType::bindStrictly)
	{
		newBounds.mMin.x = newBounds.mMax.x - newSize.x;
	}
	else //if(mBindingLeft != BindingType::bindStrictly && mBindingRight != BindingType::bindStrictly)
	{
		float sizeDiffX = (newBounds.getSize().x - newSize.x) * 0.5f;
		newBounds.mMin.x += sizeDiffX;
		newBounds.mMax.x -= sizeDiffX;
	}
	
	if(mBindingBottom == BindingType::bindStrictly)
	{
		newBounds.mMax.y = newBounds.mMin.y + newSize.y;
	}
	else if(mBindingTop == BindingType::bindStrictly)
	{
		newBounds.mMin.y = newBounds.mMax.y - newSize.y;
	}
	else //if(mBindingBottom != BindingType::bindStrictly && mBindingTop != BindingType::bindStrictly)
	{
		float sizeDiffY = (newBounds.getSize().y - newSize.y) * 0.5f;
		newBounds.mMin.y += sizeDiffY;
		newBounds.mMax.y -= sizeDiffY;
	}
	
	//assign new bounds
	
	setBounds(newBounds);
}

void Element::checkoutInitialBounds()
{
	if(mMaster == NULL) return;

	mInitialBounds			= this->getBounds();

	mInitialParentBounds	= mMaster->getBounds();
	//reset position of parent as we work in parent space
	tuple2i size_ = mInitialParentBounds.getSize();
	mInitialParentBounds.mMin = tuple2i(0,0);
	mInitialParentBounds.mMax = size_;
}

void Element::setVerticalSizing(BindingType vs)
{
	mVerticalSizing = vs;
	//updateAutoLayout();
}

void Element::setHorizontalSizing(BindingType hs)
{
	mHorizontalSizing = hs;
	//updateAutoLayout();
}

void Element::setBindingTop(BindingType bt)
{
	mBindingTop = bt;
	//updateAutoLayout();
}

void Element::setBindingBottom(BindingType bb)
{
	mBindingBottom = bb;
	//updateAutoLayout();
}

void Element::setBindingLeft(BindingType bl)
{
	mBindingLeft = bl;
	//updateAutoLayout();
}

void Element::setBindingRight(BindingType br)
{
	mBindingRight = br;
	//updateAutoLayout();
}

/*
void Element::writeTo(pugi::xml_node node)
{
	FSObjectNamed::writeTo(node);

	pugi::xml_node c_actions = objSection.append_child();
	c_actions.set_name("Actions");

	ActionCommandMap::iterator it = actionMap.begin();
	while(it!=actionMap.end())
	{
		pugi::xml_node c_action = c_actions.append_child();
		c_action.set_name("action");

		pugi::xml_attribute a_key = c_action.append_attribute("key");
		a_key.set_value( it->first.c_str() );

		pugi::xml_attribute a_command = c_action.append_attribute("command");
		a_command.set_value( it->second.c_str() );

		it++;
	}
}

void Element::readFrom(pugi::xml_node node)
{
	FSObjectNamed::readFrom(node);

	pugi::xml_node child;
	for (child = node.first_child(); child; child = child.next_sibling())
	{
		if(strcmp(child.name(),"Actions")==0)
			break;
	}
	if(child.empty()) return;

	for (pugi::xml_node c_action = child.first_child(); c_action; c_action = c_action.next_sibling())
	{
		if (strcmp(c_action.name(),"action")==0)
		{
			const pugi::xml_attribute a_key		= c_action.first_attribute();
			const pugi::xml_attribute a_command	= a_key.next_attribute();
			addAction(a_key.value(),a_command.value());
		}
	}
}
*/

}//namespace GUI { 
}//namespace Squirrel {