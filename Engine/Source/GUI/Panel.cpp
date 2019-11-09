#include "Panel.h"
#include "Render.h"
#include "Cursor.h"
#include "Manager.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Panel, GUIPanel);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Panel::Panel()
{
	mDepth=1;
	mMovable=0;
	mSizeble=0;

	SQREFL_SET_CLASS(GUI::Panel);

	wrapAtomicField("Depth", &mDepth);
	wrapAtomicField("Movable", &mMovable);
	wrapAtomicField("Sizeble", &mSizeble);

	addVerticalScroll<Slider>(SCROLL_WIDTH);
	addHorizontalScroll<Slider>(SCROLL_WIDTH);
	addSizer<Sizer>(SCROLL_WIDTH);
}

Panel::~Panel()
{
	for(int i=0; i<(int)mDepthBuffer.size(); ++i)
	{
		DELETE_PTR(mDepthBuffer[i]);
	}

	mDepthBuffer.clear();
}

void Panel::draw()
{
	Element::draw();
	drawContent();
	Render::Instance().flush();
}

void Panel::drawContent()
{
	Container::drawContent();

	if(mDepthBuffer.size()>0)
		Render::Instance().disableClipLines();

	unsigned i=0;
	for(i=0; i<mDepthBuffer.size(); ++i)
		if(mDepthBuffer[i]->getVisible())
			mDepthBuffer[i]->draw();

	ELEMENTS_LIST::iterator it = getContent().begin();
	for(; it != getContent().end(); ++it)
	{
		Element * child = (*it);
		if( child == NULL ) continue;
		if( !child->getVisible() ) continue;

		if(child->isKindOfClass("GUI::Menu"))
			static_cast<Menu *>(child)->drawMenu();
	}
}

Element * Panel::recieveEvent(EventType e, int value)
{
	Element * elem = 0;
	int i=0;

	//process menus
	ELEMENTS_LIST::iterator it = getContent().begin();
	for(; it != getContent().end(); ++it)
	{
		if( (*it) == NULL ) continue;
		if((*it)->getVisible() && (*it)->isKindOfClass("GUI::Menu"))
		{
			elem = (*it)->recieveEvent(e, value);
			if(elem) break;
		}
	}	
	
	//process sub-panels
	if(!elem)
	{
		for(i=(int)mDepthBuffer.size()-1; i>=0; --i)
		{
			if(mDepthBuffer[i]->getVisible())
			{
				elem = mDepthBuffer[i]->recieveEvent(e,value);
				if(elem) break;
			}
		}
	}

	//process itself
	if(!elem)
	{
		elem = Container::recieveEvent(e,value);
	}
	if(!elem)
	{
		elem = Element::recieveEvent(e, value);
	}
	if(elem && (e==rightDown || e==leftDown))
	{
		setTopmost();
	}
	return elem;
}

Element * Panel::get(std::string e)
{
	Element * elem = Container::get(e);
	if(!elem)	elem = getPanel(e);
	return elem;
}

void Panel::addPanel(Panel *e)
{
	if(!e) return;
	if(mDepthBuffer.size()==0)
		((Panel*)e)->setDepth(	1 );
	else
		((Panel*)e)->setDepth(	((Panel*)mDepthBuffer[mDepthBuffer.size()-1])->getDepth()+1	);
	mDepthBuffer.push_back(e);
	e->setMaster(this);
}

int Panel::numPanel()
{
	return (int)mDepthBuffer.size();
}

Panel * Panel::getPanel(int e)
{
	return (e < (int)mDepthBuffer.size()) ? (Panel*)mDepthBuffer[e] : NULL;
}

Panel * Panel::getPanel(std::string e)
{
	for(int i=0; i<(int)mDepthBuffer.size(); ++i)
		if(e==mDepthBuffer[i]->getName())
			return (Panel*)mDepthBuffer[i];
	return NULL;
}

void Panel::delPanel(Panel *p)
{
	for(int i=0; i<(int)mDepthBuffer.size(); ++i)
		if(p==mDepthBuffer[i]) {
			mDepthBuffer.erase(mDepthBuffer.begin() + i);
			return;
		}
}

Element * Panel::onLU()
{
	if(getState() == Element::stateActive)	setState( Element::stateStd );
	return this;
}

Element *Panel::onLD()
{
	setState(Element::stateActive);
	return this;
}

void Panel::resetStates()
{
	Container::resetStates();
	int i=0;
	for(i=0; i<(int)mDepthBuffer.size(); ++i)
		if(mDepthBuffer[i]->isKindOfClass("GUI::Container"))
			((Container*)mDepthBuffer[i])->resetStates();
		else
			mDepthBuffer[i]->resetState();
}

Panel * Panel::setTopmost(char *name)
{
	if(name==0 || strlen(name)<1) return 0;
	int i=0;
	for(i=(int)mDepthBuffer.size()-1; i>=0; i--)
	{
		if( strcmp(mDepthBuffer[i]->getName().c_str(),name)==0 ) 
			return setTopmost(i);
	}
	return 0;
}

Panel * Panel::setTopmost(int i)
{
	if( i<0 || i>=(int)mDepthBuffer.size() )	return 0;
	if( !mDepthBuffer[i] )					return 0;
	if( !mDepthBuffer[i]->getVisible() )		return 0;//

	Panel * last	=	(Panel*)mDepthBuffer[mDepthBuffer.size()-1];
	Panel * n		=	(Panel*)mDepthBuffer[i];
	if( n == last)	return last;
	n->   setDepth(	last->getDepth()   );
	last->setDepth(	last->getDepth()-1 );
	mDepthBuffer.erase(mDepthBuffer.begin() + i);
	mDepthBuffer.push_back(n);
	return n;
}

void Panel::setVisible(int v)
{
	Element::setVisible(v != 0);
	setTopmost();
}

void Panel::setTopmost()
{
	if(getMaster())
		if(getMaster()->isKindOfClass("GUI::Panel"))
			((Panel *)getMaster())->setTopmost((char*)getName().c_str());
}

}//namespace GUI { 
}//namespace Squirrel {