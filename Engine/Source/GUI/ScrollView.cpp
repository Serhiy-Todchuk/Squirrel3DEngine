// ScrollView.cpp: implementation of the ScrollView class.
//
//////////////////////////////////////////////////////////////////////

#include "ScrollView.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

#define MAX_RESOLUTION 5000

std::vector<Math::vec4>	ScrollView::cs;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ScrollView::ScrollView()
{
	SQREFL_SET_CLASS(GUI::ScrollView);
	horizontalScroll = 0;
	verticalScroll = 0;
	sizer = 0;
	scrollsOffset = tuple2i(0,0);
	maxScrollsOffset = tuple2i(0,0);
	Element::setSize(tuple2i(96,96));
	mMargin = tuple4i(0,0,0,0);
}

ScrollView::~ScrollView()
{
}

void ScrollView::draw()
{
	Element::draw();
	drawContent();
}

void ScrollView::beginDrawContent()
{
	float up	= getConstraintUp() + mMargin.y;
	float down	= getConstraintDown() - mMargin.w;
	float left	= getConstraintLeft() + mMargin.x;
	float right	= getConstraintRight() - mMargin.z;

	Math::vec4 c = Math::vec4(up,down,left,right);//constraits

	cs.push_back(c);

	setConstraints(c);
}

void ScrollView::endDrawContent()
{
	cs.pop_back();
	if(cs.size())
	{
		setConstraints(cs[cs.size()-1]);
	}
	else
	{
		Render::Instance().disableClipLines();
	}

	if(getDrawStyle() > 0)
	{
		if(sizer) sizer->draw();
		if(verticalScroll) if(verticalScroll->getVisible()) verticalScroll->draw();
		if(horizontalScroll) if(horizontalScroll->getVisible()) horizontalScroll->draw();
	}
}

Element * ScrollView::recieveEvent(EventType e, int value)
{
	Element * elem = 0;
	if(verticalScroll) if(verticalScroll->getVisible())
	{
		elem = verticalScroll->recieveEvent(e,value);
		if(elem) return elem;
	}
	if(horizontalScroll) if(horizontalScroll->getVisible())
	{
		elem = horizontalScroll->recieveEvent(e,value);
		if(elem) return elem;
	}
	if(sizer && sizer->getVisible())
	{
		elem = sizer->recieveEvent(e,value);
		if(elem) return elem;
	}

	return NULL;
}

float ScrollView::getConstraintUp()
{
	float up = 0;
	if(cs.size()>0) up = cs[cs.size()-1].x;
	float myUp = (float) getGlobalPos().y;
	return Math::maxValue(up,myUp);
}

float ScrollView::getConstraintDown()
{
	float down = MAX_RESOLUTION;
	if(cs.size()>0) down = cs[cs.size()-1].y;
	float myDown = (float) getGlobalPos().y + getSize().y;
	return Math::minValue(down,myDown);
}

float ScrollView::getConstraintLeft()
{
	float left = 0;
	if(cs.size()>0) left = cs[cs.size()-1].z;
	float myLeft = (float) getGlobalPos().x;
	return Math::maxValue(left,myLeft);
}

float ScrollView::getConstraintRight()
{
	float right = MAX_RESOLUTION;
	if(cs.size()>0) right = cs[cs.size()-1].w;
	float myRight = (float) getGlobalPos().x + getSize().x;
	return Math::minValue(right,myRight);
}

void	ScrollView::setConstraints(Math::vec4 c)
{
	Render::Instance().enableClipLines(c.x, c.y, c.z, c.w);
}

void ScrollView::deleteHorizontalScroll()
{
	DELETE_PTR(horizontalScroll);
	horizontalScroll = NULL;
	updateScrolls();
}

void ScrollView::deleteVerticalScroll()
{
	DELETE_PTR(verticalScroll);
	verticalScroll = NULL;
	updateScrolls();
}

bool ScrollView::hasVerticalScroll()
{
	if(!verticalScroll)
		return false;

	return verticalScroll->getVisible();
}

bool ScrollView::hasHorizontalScroll()
{
	if(!horizontalScroll)
		return false;

	return horizontalScroll->getVisible();
}

void ScrollView::updateScrolls()
{
	tuple2i contentSize = getContentSize();

	tuple2i visibleAreaSize = getSize();
	visibleAreaSize.x -= mMargin.x + mMargin.z;
	visibleAreaSize.y -= mMargin.y + mMargin.w;

	contentSize.x = Math::maxValue(getSize().x, contentSize.x);
	contentSize.y = Math::maxValue(getSize().y, contentSize.y);

	bool vVis = verticalScroll		&& contentSize.y > getSize().y;
	bool hVis = horizontalScroll	&& contentSize.x > getSize().x;
	if(vVis)	contentSize.x += verticalScroll->getSize().x;
	if(hVis)	contentSize.y += horizontalScroll->getSize().y;

	if(verticalScroll)
	{
		if(vVis)
		{
			verticalScroll->setVisible(1);
			verticalScroll->setPosSep(getSize().x-scrollsWidths.y, 0);
			verticalScroll->setSizeSep(scrollsWidths.y, getSize().y - (hVis || sizer ? scrollsWidths.x : 0) );
			verticalScroll->setPointSize((float)getSize().y / contentSize.y);
			maxScrollsOffset.y = contentSize.y - getSize().y;
			scrollsOffset.y = (int)(maxScrollsOffset.y * verticalScroll->getValue());
		}
		else
		{
			//verticalScroll->setValue(0);
			verticalScroll->setVisible(0);
			maxScrollsOffset.y = 0;
			scrollsOffset.y = 0;
		}
	}
	if(horizontalScroll)
	{
		if(hVis)
		{
			horizontalScroll->setVisible(1);
			horizontalScroll->setPosSep(0, getSize().y-scrollsWidths.x);
			horizontalScroll->setSizeSep(getSize().x - (vVis || sizer ? scrollsWidths.x : 0), scrollsWidths.y );
			horizontalScroll->setPointSize((float)getSize().x / contentSize.x);
			maxScrollsOffset.x = contentSize.x - getSize().x;
			scrollsOffset.x = (int)(maxScrollsOffset.x * horizontalScroll->getValue());
		}
		else
		{
			//horizontalScroll->setValue(0);
			horizontalScroll->setVisible(0);
			maxScrollsOffset.x = 0;
			scrollsOffset.x = 0;
		}
	}

	if((verticalScroll && vVis) || (horizontalScroll && hVis))
	{
		if(!sizer)
		{
			addSizer<Sizer>(SCROLL_WIDTH);
			sizer->setDragable(0);
			sizer->setVisible(1);
		}
	}
	else if(sizer && sizer->getDragable()==0)
	{
		sizer->setVisible(0);
	}

	if(sizer)
	{
		sizer->setPos(getSize() - sizer->getSize());
	}

	//update master pos of children
	setPos(getPos());
}

void ScrollView::resetScrolls()
{
	scrollsOffset = tuple2i(0, 0);

	if(verticalScroll != NULL && verticalScroll->getVisible())
	{
		verticalScroll->setValue(0);
	}
	if(horizontalScroll != NULL && horizontalScroll->getVisible())
	{
		horizontalScroll->setValue(0);
	}

	//update master pos of children
	setPos(getPos());
}


void ScrollView::resetState()
{
	Element::resetState();
	if(verticalScroll)		verticalScroll->resetState();
	if(horizontalScroll)	horizontalScroll->resetState();
	if(sizer)				sizer->resetState();
}

void ScrollView::setSize(tuple2i size)
{
	Element::setSize(size);
	if(sizer) sizer->setPos(getSize() - scrollsWidths);
	updateScrolls();
}

void ScrollView::setPos(tuple2i pos)
{
	Element::setPos(pos);
	if(verticalScroll)		verticalScroll->setMasterPos( getGlobalPos() );
	if(horizontalScroll)	horizontalScroll->setMasterPos( getGlobalPos() );
	if(sizer)				sizer->setMasterPos( getGlobalPos() );
}

//ActionDelegate definition
bool ScrollView::processAction(const std::string& action, Element * sender) 
{
	if(verticalScroll && verticalScroll==sender)
	{
		scrollsOffset.y = (int)(maxScrollsOffset.y * verticalScroll->getValue());
		//update master pos of children
		setPos(getPos());
		return true;
	}
	if(horizontalScroll && horizontalScroll==sender)
	{
		scrollsOffset.x = (int)(maxScrollsOffset.x * horizontalScroll->getValue());
		//update master pos of children
		setPos(getPos());
		return true;
	}
	return Element::processAction(action,sender);
}

void ScrollView::deleteSizer()
{
	if(sizer)
	{
		delete sizer;
		sizer = 0;
	}
}

Element * ScrollView::onMW(float delta)
{
	float contentSizeY = (float)getContentSize().y;
	float sizeY = (float)getSize().y;
	bool vVis = verticalScroll		&& contentSizeY > sizeY;
	if(vVis)
	{
		float scrollValueKoef = (float)sizeY / maxScrollsOffset.y;
		verticalScroll->setValue(verticalScroll->getValue() - delta * scrollValueKoef * 0.05f);
		scrollsOffset.y = (int)(maxScrollsOffset.y * verticalScroll->getValue());
		//update master pos of children
		setPos(getPos());
		return this;
	}
	return NULL;
}

}//namespace GUI { 
}//namespace Squirrel {