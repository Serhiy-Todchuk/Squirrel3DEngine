// Panel.cpp: implementation of the Panel class.
//
//////////////////////////////////////////////////////////////////////

#include "Container.h"
#include "Render.h"
#include "Cursor.h"
#include "Manager.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Container, GUIContainer);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Container::Container()
{
	mMargin = tuple4i(2,2,2,2);
	setSize(tuple2i(96,96));

	SQREFL_SET_CLASS(GUI::Container);

	wrapAtomicField("margin", &mMargin.x, 4);
	wrapCollectionField<ELEMENTS_LIST, Element>("content", &mElems);
}

Container::~Container()
{
	clear();
}

tuple2i	Container::getContentSize()
{
	tuple2i contentSize(0, 0);
	tuple2i leftTopMargin(mMargin.x, mMargin.y);

	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) == NULL ) continue;
		if( !(*it)->getVisible() ) continue;
		tuple2i bottomRight = (*it)->getPos() + (*it)->getSize() + leftTopMargin;
		contentSize.x = Math::maxValue(bottomRight.x, contentSize.x);
		contentSize.y = Math::maxValue(bottomRight.y, contentSize.y);
	}

	contentSize.x += mMargin.z;
	contentSize.y += mMargin.w;

	return contentSize;
}

void Container::drawContent()
{
	beginDrawContent();

	tuple2i masterPos = getGlobalPos() - getScrollsOffset() + tuple2i(mMargin.x, mMargin.y);

	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		Element * child = (*it);
		if( child == NULL ) continue;
		if( !child->getVisible() ) continue;
		if( !isIntersect(masterPos + child->getPos(), child->getSize()) ) continue;

		child->draw();
	}

	endDrawContent();
}

void Container::setPos(tuple2i pos)
{
	ScrollView::setPos(pos);
	tuple2i leftTopMargin(mMargin.x, mMargin.y);
	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		(*it)->setMasterPos(getGlobalPos() - getScrollsOffset() + leftTopMargin);
		//updateAutoLayout called in setMasterPos so no need to call it here
	}
}

void Container::setSize(tuple2i size)
{
	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		(*it)->updateAutoLayout(size);
	}
	ScrollView::setSize(size);
}

void Container::deserialize(Reflection::Deserializer * deserializer)
{
	Object::deserialize(deserializer);

	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		Element * child = (*it);
		if( child == NULL ) continue;
		child->setMaster(this);
	}

	updateScrolls();
}

Element *Container::add(Element *e)
{
	if(!e) return 0;
	mElems.push_back(e);
	e->setMaster(this);
	updateScrolls();
	return e;
}

void Container::resetStates()
{
	ScrollView::resetState();
	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) == NULL ) continue;
		(*it)->resetState();
	}
}

void Container::clear()
{
	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		DELETE_PTR(*it);
	}
	mElems.clear();
	updateScrolls();
}

Element * Container::get(std::string name)
{
	ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) == NULL ) continue;
		if(name == (*it)->getName())
			return (*it);
	}
	return NULL;
}

void Container::del(std::string name)
{
	ELEMENTS_LIST::iterator it = mElems.begin();
	while(it != mElems.end())
	{
		if(name == (*it)->getName())
		{
			it = mElems.erase(it);
			updateScrolls();
			break;
		}
		else
		{
			++it;
		}
	}
}

void Container::del(Element * e)
{
	ELEMENTS_LIST::iterator it = mElems.begin();
	while(it != mElems.end())
	{
		if(e == (*it))
		{
			it = mElems.erase(it);
			updateScrolls();
			break;
		}
		else
		{
			++it;
		}
	}
}

Element * Container::recieveEvent(EventType e, int value)
{
	Element * elem = ScrollView::recieveEvent(e, value);

	if(elem == NULL)
	{
		ELEMENTS_LIST::iterator it = mElems.begin();
		for(; it != mElems.end(); ++it)
		{
			if( (*it) == NULL ) continue;
			if((*it)->getVisible())
			{
				elem = (*it)->recieveEvent(e,value);
				//if(e==mouseMove) updateScrolls();
				if(elem) break;
			}
		}
	}

	if(elem == NULL)
	{
		elem = Element::recieveEvent(e, value);
	}

	return elem;
}

}//namespace GUI { 
}//namespace Squirrel {