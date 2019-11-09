#include "Manager.h"
#include "Cursor.h"
#include "Render.h"
#include <Common/Input.h>
#include <Render/IRender.h>

#ifdef _WIN32
# include "WindowsFontGenerator.h"
#elif __APPLE__
# include "MacFontGenerator.h"
#endif

namespace Squirrel {
namespace GUI { 

Manager::Manager() 
{
	mMainFont = NULL;
	mState = stateIdle;
	mEatInput = true;
	mRootMenu = NULL;
	mMainPanel = NULL;
}

Manager::~Manager() 
{
	DELETE_PTR(mRootMenu);
}

Manager& Manager::Instance()
{
	static Manager instance;
	return instance;
}

void Manager::init()
{
	//create fonts
	FontGenerator * fontGenerator = NULL;
	
#ifdef _WIN32
	fontGenerator = new WindowsFontGenerator;
#elif __APPLE__
	fontGenerator = new MacFontGenerator;
#endif

	Font * smallFont	= fontGenerator->create("Terminus", 6, Font::NORMAL);
	Font * normalFont	= fontGenerator->create("Terminus", 8, Font::NORMAL);
	Font * bigFont		= fontGenerator->create("Terminus", 16, Font::NORMAL);

	Render::Instance().setFont(Render::sizeSmall,	smallFont);
	Render::Instance().setFont(Render::sizeNormal,	normalFont);
	Render::Instance().setFont(Render::sizeBig,		bigFont);

	mMainFont = normalFont;

	delete fontGenerator;

	mRootMenu = new Menu();

	mMainPanel = new Panel();

	//clean main panel
	mMainPanel->clear();
	mMainPanel->add(mRootMenu);
	mRootMenu->setVisible(false);

	//Reflection::ObjectCreator 
}

void Manager::showMenu(MenuContentSource * menuSrc, tuple2i pos)
{
	mRootMenu->setContentSource(menuSrc);
	mRootMenu->setVisible(true);
	mRootMenu->setMenuLayout(pos, mDefaultMenuWidth, false);
	mRootMenu->deselect();
	mRootMenu->resetScrolls();
}

void Manager::showMenu(MenuContentSource * menuSrc)
{
	showMenu(menuSrc, Cursor::Instance().getPos());
}

void Manager::update()
{
	mState = stateProcessInput;

	//handle mouse movement
	if(Input::Get()->isMouseMoved())
	{
		Cursor::Instance().setPos( Input::Get()->getMousePos() );
		onMM();
	}

	//handle keys downs
	Input::KEYS_QUEUE * keysDownQueue = Input::Get()->getDownKeysQueue();
	if(keysDownQueue->size())
	{
		//LMB down
		if(Input::Get()->isKeyDown(Input::Mouse0))
		{
			onLD();
		}

		//RMB down
		if(Input::Get()->isKeyDown(Input::Mouse1))
		{
			onRD();
		}

		//other keys downs
		for(Input::KEYS_QUEUE::iterator itKeyDown = keysDownQueue->begin();
			itKeyDown != keysDownQueue->end(); ++itKeyDown)
		{
			if((*itKeyDown) == Input::Mouse0 || (*itKeyDown) == Input::Mouse1)
				continue; //skip LMB and RMB as they are handled separatelly 
			onKD((*itKeyDown));
		}
	}

	//handle keys ups
	Input::KEYS_QUEUE * keysUpQueue = Input::Get()->getUpKeysQueue();
	if(keysUpQueue->size())
	{
		//LMB up
		if(Input::Get()->isKeyUp(Input::Mouse0))
		{
			onLU();
		}

		//RMB up
		if(Input::Get()->isKeyUp(Input::Mouse1))
		{
			onRU();
		}

		//other keys ups
		Input::KEYS_QUEUE::iterator itKeyUp = keysUpQueue->begin();
		for(; itKeyUp != keysUpQueue->end(); ++itKeyUp)
		{
			if((*itKeyUp) == Input::Mouse0 || (*itKeyUp) == Input::Mouse1)
				continue; //skip LMB and RMB as they are handled separatelly 
			onKU((*itKeyUp));
		}
	}

	//handle characters
	std::string inputStr = Input::Get()->getInputString();
	if(inputStr.size() > 0)
	{
		for(uint i = 0; i < inputStr.size(); ++i)
		{
			onCh( inputStr[i] );		
		}
	}

	//handle mouse wheel scrolling
	if(Input::Get()->getMouseScroll() != 0)
	{
		if(onMW( Input::Get()->getMouseScroll() ) && mEatInput)
		{
			Input::Get()->setMouseScroll(0);
		}
	}

	//handle left double-click
	if(Input::Get()->isDoubleClicked())
	{
		onDBL();
	}

	mState = stateIdle;
}

void Manager::render()
{
	mState = stateRender;

	mMainPanel->setDrawStyle(0);
	if(mMainPanel->getVisible())
	{
		mMainPanel->setSize(Squirrel::Render::IRender::GetActive()->getWindow()->getSize());
		mMainPanel->draw();
	}

	if(mDraggingElem) mDraggingElem->draw();

	Render::Instance().finish();

	mState = stateIdle;
}

void Manager::onLU()
{
	Element * e = 0;
	if(mDraggingElem) e = mDraggingElem->recieveEvent(Element::mouseMove,0);
	if(!e)	e = mMainPanel->recieveEvent(Element::leftUp,0);

	if( e && (e == Element::GetFocus()) ) 
	{
		if(e == mDraggingElem)
		{
			Element::SetFocus(0);
			Element * e2 = 0;
			e2 = mMainPanel->recieveEvent(Element::checkUnderMouse,0);
			bool result = false;
			if(e2)	result = e2->acceptDrag(e);
			if(!result)
			{
				delete e;//TODO: wtf?
			}
		}
		else
		{
			e->onLClk();
		}
	}

	mLMBClicked = 0;
	mDraggingElem = 0;
	mMainPanel->resetStates();
	Cursor::Instance().setState(Cursor::tNormal);
}

bool Manager::onLD()
{
	if(	!mRootMenu->isPointIn( Cursor::Instance().getPos() ) )
		mRootMenu->setVisible(false);

	Element * e = 0;
	e = mMainPanel->recieveEvent(Element::leftDown,0);
	Element::SetFocus( e );
	mLMBClicked = e;

	return e != NULL;
}

void Manager::onRU()
{
	Element * e = mMainPanel->recieveEvent(Element::rightUp,0);

	if( e && (e == Element::GetFocus()) ) 
		e->onRClk();
}

void Manager::onRD()
{
	if(	!mRootMenu->isPointIn( Cursor::Instance().getPos() ) )
		mRootMenu->setVisible(false);

	Element::SetFocus( mMainPanel->recieveEvent(Element::rightDown,0) );
}

void Manager::onMM()
{
	Element * e = 0;

	//send move event to drag elem and if it accepts - process with it
	if(mDraggingElem) e = mDraggingElem->recieveEvent(Element::mouseMove,0);

	//send event to all elements
	if(e == NULL)	e = mMainPanel->recieveEvent(Element::mouseMove, 0);

	//process dragging
	if( e && (e == Element::GetFocus()) )
	{
		if(!mDraggingElem && e == mLMBClicked)
		{
			mDraggingElem = e->beginDrag();
			if(mDraggingElem)
			{
				mDraggingElem->setPos(mDraggingElem->getGlobalPos());
				Element * m = mDraggingElem->getMaster();
				if(m && m->isKindOfClass("GUI::Container"))
					((Container*)m)->del( mDraggingElem );
				Element::SetFocus( mDraggingElem );
			}
		}
		if(mDraggingElem)
		{
			tuple2i dpos = mDraggingElem->getGlobalPos() + Cursor::Instance().getPosDiff();
			mDraggingElem->setPos(dpos);
		}
	}
}

void Manager::onCh(char_t ch)
{
	mMainPanel->recieveEvent(Element::keyChar,(int)ch);
}

void Manager::onKD(Input::NamedKey key)
{
	mMainPanel->recieveEvent(Element::keyDown,(int)key);
}

void Manager::onKU(Input::NamedKey key)
{
	mMainPanel->recieveEvent(Element::keyUp,(int)key);
}

bool Manager::onMW(float delta)
{
	return mMainPanel->recieveEvent(Element::mouseWheel,(int)delta) != NULL;
}

void Manager::onDBL()
{
	mMainPanel->recieveEvent(Element::leftDouble,0);
}



}//namespace GUI { 
}//namespace Squirrel {
