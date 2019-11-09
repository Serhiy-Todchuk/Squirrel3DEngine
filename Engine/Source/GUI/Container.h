
#pragma once

#include "ScrollView.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Container  : public ScrollView
{
public:

	typedef		std::list<Element *> ELEMENTS_LIST;


private:

	ELEMENTS_LIST mElems;

protected:

	//implement scrollView
	virtual tuple2i		getContentSize();
	virtual void		drawContent();

	ELEMENTS_LIST& getContent() { return mElems; }

public:

	Container();
	virtual ~Container();

	void setMargin(tuple4i margin) { mMargin = margin; }
	const tuple4i& getMargin() const { return mMargin; }
	
	const ELEMENTS_LIST& getContent() const { return mElems; }
	virtual Element *add(Element * e);
	Element * get(std::string name);
	void del(Element * e);
	void del(std::string e);
	void clear();

	virtual void setPos(tuple2i pos);
	virtual void setSize(tuple2i size);

	virtual Element * recieveEvent(EventType e, int value);

	virtual void resetStates();

	virtual void deserialize(Reflection::Deserializer * deserializer);

	//method creates UIElem from template
	template<class _TElem>
	_TElem * add(const char * name, int posx, int posy, int sizex, int sizey)
	{
		_TElem * ret = new _TElem;
		ret->setName(name);
		ret->setText(name);
		ret->setPosSep (posx,  posy);
		ret->setSizeSep(sizex, sizey);
		ret->setVisible(1);
		add(ret);
		return ret;
	}
};
}//namespace GUI { 
}//namespace Squirrel {