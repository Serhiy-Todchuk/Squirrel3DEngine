#pragma once

#include "Container.h"

namespace Squirrel {
namespace GUI { 


class SQGUI_API Panel  : public Container
{
	typedef		std::vector<Element *> ELEMENTS_ARR;

	int  mDepth;
	bool mMovable;
	bool mSizeble;

	ELEMENTS_ARR mDepthBuffer;

	void setDepth(int d)	{ mDepth = d; }
public:
	Panel();
	virtual ~Panel();

	//void add(Element *e);
	Element * get(std::string e);

	void addPanel(Panel *e);
	int numPanel();
	Panel * getPanel(int e);
	Panel * getPanel(std::string e);
	void delPanel(Panel *p);

	void setTopmost();
	Panel * setTopmost(int i);
	Panel * setTopmost(char *name);

	void setVisible(int v);

	bool isMovable()		{ return mMovable; }
	bool isSizeble()		{ return mSizeble; }
	int getDepth()			{ return mDepth; }
	void setSizeble(bool s)	{ mSizeble = s; }
	void setMovable(bool m)	{ mMovable = m; }

	virtual void draw();
	virtual void resetStates();

	Element * recieveEvent(EventType e, int value);

	virtual Element * onLU();
	virtual Element * onLD();

protected:
	void drawContent();

};

}//namespace GUI { 
}//namespace Squirrel {