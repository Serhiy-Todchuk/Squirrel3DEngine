// Element.h: interface for the Element class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <map>
#include <common/common.h>
#include <Reflection/CollectionWrapper.h>
#include <Reflection/AtomicWrapper.h>
#include <Reflection/Object.h>
#include "Bounds2D.h"
#include "Render.h"
#include "macros.h"

namespace Squirrel {
namespace GUI { 

using namespace Math;

class Element;

class SQGUI_API ActionHandler
{
public:
	ActionHandler() {}
	virtual ~ActionHandler() {}

	// key method
	virtual void execute(const std::string& action, Element * sender) = 0;

	virtual ActionHandler * clone() = 0;
};

template <class _T>
class ActionFunctor: public ActionHandler 
{
	_T * mObject;
	void (_T::*mMethod)(const std::string&, Element *);
	void (_T::*mSimpleMethod)();

public:
	ActionFunctor(_T * obj, void (_T::*met)()):
		mObject(obj), mSimpleMethod(met), mMethod(NULL) {}
	ActionFunctor(_T * obj, void (_T::*met)(const std::string&, Element *)):
		mObject(obj), mSimpleMethod(NULL), mMethod(met) {}
	virtual ~ActionFunctor() {}

	// key method
	virtual void execute(const std::string& action, Element * sender) {
		if(mSimpleMethod)
			(*mObject.*mSimpleMethod)();
		else
			(*mObject.*mMethod)(action, sender);
	}

	virtual ActionHandler * clone() {
		if(mSimpleMethod)
			return new ActionFunctor<_T>(mObject, mSimpleMethod);
		return new ActionFunctor<_T>(mObject, mMethod);
	}
};

class SQGUI_API ActionDelegate
{
public:
	ActionDelegate() {}
	~ActionDelegate() {}

	virtual bool processAction(const std::string& action, Element * sender) = 0;
};

typedef std::map<std::string, ActionHandler *>	ActionHandlersMap;

// Element

class SQGUI_API Element : 
	public Reflection::Object, 
	public ActionDelegate
{
public:

	static const char_t * SIZE_CHANGED_ACTION;
	static const char_t * POSITION_CHANGED_ACTION;
	static const char_t * STATE_CHANGED_ACTION;
	static const char_t * VALUE_CHANGED_ACTION;
	
public:

	enum State
	{
		stateActive = 0,
		stateStd = 1,
		stateOver = 2,
	};

	enum TextAlignment
	{
		textLeft = 0,
		textCenter,
		textRight,
	};

	enum EventType
	{
		keyUp = 1,
		keyDown,
		leftUp,
		leftDown,
		rightUp,
		rightDown,
		mouseMove,
		keyChar,
		leftDouble,
		mouseWheel,
		checkUnderMouse,
	};

	enum BindingType
	{
		bindStrictly,
		bindFlexible
	};

	Element();
	virtual ~Element();

	static bool		IsIn(tuple2i min_, tuple2i max_, tuple2i pt);
	tuple2i			getGlobalPos();
	virtual bool	isIn(tuple2i pt);
	virtual int		isIntersect(tuple2i pos, tuple2i size);
	virtual void	resetState() { mState = stateStd;}
	virtual void	draw();

	void updateAutoLayout(tuple2i parentSize);
	void checkoutInitialBounds();
	
	//	<accessors>
	
	Element *			getMaster() 				{ return mMaster; }
	const std::string&	getText() 			const	{ return mText; }
	const std::string&	getHelp()			const	{ return mHelp; }
	int					getId()				const	{ return mId; }
	int					getDrawStyle()		const	{ return mDrawStyle; }
	bool				getVisible()		const	{ return mVisible; }
	bool				getDragable()		const	{ return mDragable; }
	tuple2i				getPos()			const	{ return mPos; }
	tuple2i				getSize()			const	{ return mSize; }
	State				getState() 			const	{ return mState; }
	Render::Size		getFontSize() 		const	{ return mFontSize; }
	TextAlignment		getTextAlignment()	const	{ return mTextAlignment; }

	BindingType getVerticalSizing()		const	{ return mVerticalSizing; }
	BindingType getHorizontalSizing()	const	{ return mHorizontalSizing; }

	BindingType getBindingTop()			const	{ return mBindingTop; }
	BindingType getBindingBottom()		const	{ return mBindingBottom; }
	BindingType getBindingLeft()		const	{ return mBindingLeft; }
	BindingType getBindingRight()		const	{ return mBindingRight; }


	void	setMaster(Element * m);
	void	setMasterPos(tuple2i masterPos);
	void	setDrawStyle(int s);
	void	setDragable(bool d);
	void	setState(State s);
	void	setPosSep(int x, int y);
	void	setSizeSep(int x, int y);
	void	setTextAlignment(TextAlignment align);
	void	setFontSize(Render::Size size);
	void	setId(int id);
	void	setHelp(const std::string& help);
	virtual void	setText(const std::string& text);
	virtual void	setPos(tuple2i pos);
	virtual void	setSize(tuple2i size);
	virtual void	setVisible(bool v);

	void setVerticalSizing(BindingType vs);
	void setHorizontalSizing(BindingType hs);

	void setBindingTop(BindingType bt);
	void setBindingBottom(BindingType bb);
	void setBindingLeft(BindingType bl);
	void setBindingRight(BindingType br);

	//	<accessors/>

	//drag reciever methods
	virtual bool canRecieveDrag(Reflection::Object * drag)	{ return false; }
	virtual bool acceptDrag(Reflection::Object * drag)		{ return false; }

	//self drag methods
	virtual Element * beginDrag()	{ return 0; }

	//	<action handlers>
	virtual Element * onBeginDrag()	{return this;};
	virtual Element * onEndDrag()	{return this;};
	virtual Element * onLClk()		{return this;};
	virtual Element * onRClk()		{return this;};
	virtual Element * onLDbl()		{return this;};
	virtual Element * onFocus()		{return this;};
	virtual Element * onUnFocus()	{return this;};
	//	<action handlers/>

	// process event method
	virtual Element * recieveEvent(EventType e, int value);

	bool doesAcceptEvent(EventType e);

	//	<action delegate accessors>
	ActionDelegate * getActionDelegate() { return mActionDelegate; }
	void setActionDelegate(ActionDelegate * ad) { mActionDelegate = ad; }
	//	<action delegate accessors/>

	//	<action management methods>
	void addAction(const std::string& action, ActionHandler * handler);
	bool executeAction(const std::string& action);
	void clearActions();

	template <class _T>
	void addAction(const std::string& action, _T * obj, void (_T::*met)(const std::string&, Element *))	{
		addAction(action, new ActionFunctor<_T>(obj, met));
	}
	template <class _T>
	void addAction(const std::string& action, _T * obj, void (_T::*met)())	{
		addAction(action, new ActionFunctor<_T>(obj, met));
	}
	//	<action management methods/>

	//	<focus management methods>
	void setFocus();
	static void ResetFocus();
	static Element * GetFocus();
	static void SetFocus(Element * elem);
	//	<focus management methods/>

	//ActionDelegate declaration
	virtual bool processAction(const std::string& action, Element * sender);

protected:

	// action->command map
	ActionHandlersMap mActionMap;

	// action delegate
	ActionDelegate * mActionDelegate;

	//	<event handlers>
	virtual Element * onLU()		{return NULL;};
	virtual Element * onLD()		{return NULL;};
	virtual Element * onRU()		{return NULL;};
	virtual Element * onRD()		{return NULL;};
	virtual Element * onMM()		{return NULL;};
	virtual Element * onCh(char ch) {return NULL;};
	virtual Element * onKD(int key) {return NULL;};
	virtual Element * onDBL()		{return NULL;};
	virtual Element * onMW(float delta) {return NULL;};
	//	<event handlers/>

	void clipContent();
	void unclipContent();

private:

	Bounds2D getBounds() const { return Bounds2D( getPos(), getSize().x, getSize().y); }
	void setBounds(Bounds2D bounds)
	{
		setPos( bounds.mMin );
		setSize( bounds.getSize() );
	}

private:
	//focused element
	static Element * sActive;

	State		mState;
	int			mDrawStyle;
	bool		mVisible;
	bool		mDragable;
	TextAlignment	mTextAlignment;
	Render::Size	mFontSize;

	std::string	mName;
	std::string	mText;
	std::string	mHelp;
	tuple2i	mPos;
	tuple2i	mSize;
	int mId;

	Element * mMaster;

	//cached master pos
	tuple2i	mMasterPos;

	BindingType mVerticalSizing;
	BindingType mHorizontalSizing;

	BindingType mBindingTop;
	BindingType mBindingBottom;
	BindingType mBindingLeft;
	BindingType mBindingRight;

	Bounds2D mInitialBounds;
	Bounds2D mInitialParentBounds;

	Math::vec4 mStoredClipLines;
};

inline void	Element::setId(int id)
{
	mId = id;
}

inline void	Element::setMasterPos(tuple2i masterPos)
{
	mMasterPos = masterPos;
	setPos(mPos);//update position
}

inline	void	Element::setMaster(Element * m)			
{ 
	mMaster = m; 
	checkoutInitialBounds();
}

inline	void	Element::setDrawStyle(int s)				
{ 
	mDrawStyle = s; 
}

inline	void	Element::setVisible(bool v)				
{ 
	mVisible = v; 
}

inline	void	Element::setDragable(bool d)				
{ 
	mDragable = d; 
}

inline	void	Element::setState(State s)				
{ 
	bool triggerAction = mState != s;
	mState = s;
	if(triggerAction)
		processAction(STATE_CHANGED_ACTION, this);
}

inline	void	Element::setText(const std::string& text)				
{ 
	mText = text; 
}

inline	void	Element::setHelp(const std::string& help)				
{ 
	mHelp = help; 
}

inline	void	Element::setPos(tuple2i pos)		
{ 
	bool triggerPosChangedAction = mPos != pos;
	mPos = pos; 
	//checkoutInitialBounds();
	if(triggerPosChangedAction)
		processAction(POSITION_CHANGED_ACTION, this);
}

inline	void	Element::setSize(tuple2i size)	
{ 
	bool triggerSizeChangedAction = mSize != size;
	mSize = size; 
	//checkoutInitialBounds();
	if(triggerSizeChangedAction)
		processAction(SIZE_CHANGED_ACTION, this);
}

inline	void	Element::setPosSep(int x, int y)			
{ 
	setPos(tuple2i(x,y)); 
}

inline	void	Element::setSizeSep(int x, int y)		
{ 
	setSize(tuple2i(x,y)); 
}

inline	void	Element::setTextAlignment(TextAlignment align) 
{ 
	mTextAlignment = align; 
}	

inline	void	Element::setFontSize(Render::Size size)
{ 
	mFontSize = size; 
}

class SQGUI_API ElemFactory
{

public:
	ElemFactory() {}
	virtual ~ElemFactory() {}

	//method creates UIElem from template
	template<class _TElem>
	_TElem * createElem(const char_t * name, int posx, int posy, int sizex, int sizey)
	{
		_TElem * ret = new _TElem;
		ret->setName(name);
		ret->setText(name);
		ret->setPosSep (posx,  posy);
		ret->setSizeSep(sizex, sizey);
		ret->setVisible(1);
		return ret;
	}

	static ElemFactory& Instance();
};


}//namespace GUI { 
}//namespace Squirrel {