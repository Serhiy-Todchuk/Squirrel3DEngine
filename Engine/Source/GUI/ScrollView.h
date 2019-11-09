
#pragma once

#include "Element.h"
#include "Slider.h"
#include "Sizer.h"
#include <Math/mathTypes.h>

namespace Squirrel {
namespace GUI {

class SQGUI_API ScrollView  : public Element
{
public:

	static const int SCROLL_WIDTH = 8;

private:
	tuple2i scrollsWidths;
	tuple2i scrollsOffset;
	tuple2i maxScrollsOffset;
	Slider * verticalScroll;
	Slider * horizontalScroll;

	Sizer * sizer;

	void setConstraints(Math::vec4 c);

	float getConstraintUp();
	float getConstraintDown();
	float getConstraintLeft();
	float getConstraintRight();

	static std::vector<Math::vec4> cs;//constraitsStack

protected:

	tuple4i mMargin;

protected:

	virtual tuple2i		getContentSize()	= 0;
	virtual void		drawContent()		= 0;

	void beginDrawContent();//TODO: make private
	void endDrawContent();//TODO: make private

	virtual Element * onMW(float delta);

	template<class _TSizer>
	void addSizer(int width)
	{
		sizer = ElemFactory::Instance().createElem<_TSizer>("sizer", getSize().x-width, getSize().y-width, width, width);
		sizer->setDrawStyle(0);
		sizer->setMaster(this);
	}
	template<class _TScroll>
	void addVerticalScroll(int width)
	{
		verticalScroll = ElemFactory::Instance().createElem<_TScroll>("verticalScroll", getSize().x-width, 0, width, getSize().y - width);
		verticalScroll->setVertical(true);
		verticalScroll->setActionDelegate(this);
		verticalScroll->setMaster(this);
		scrollsWidths.y = width;
		updateScrolls();
	}
	template<class _TScroll>
	void addHorizontalScroll(int width)
	{
		horizontalScroll = ElemFactory::Instance().createElem<_TScroll>("horizontalScroll", 0, getSize().y-width, getSize().x - width, width);
		horizontalScroll->setMaster(this);
		horizontalScroll->setActionDelegate(this);
		scrollsWidths.x = width;
		updateScrolls();
	}

	void deleteHorizontalScroll();
	void deleteVerticalScroll();
	void deleteSizer();

	inline tuple2i getScrollsOffset() { return scrollsOffset; }

public:

	ScrollView();
	virtual ~ScrollView();

	void updateScrolls();
	void resetScrolls();

	bool hasVerticalScroll();
	bool hasHorizontalScroll();

	virtual void draw();

	virtual void setSize(tuple2i size);
	virtual void setPos(tuple2i pos);

	virtual void resetState();

	virtual Element * recieveEvent(EventType e, int value);
	virtual bool processAction(const std::string& action, Element * sender);
};

}//namespace GUI { 
}//namespace Squirrel {