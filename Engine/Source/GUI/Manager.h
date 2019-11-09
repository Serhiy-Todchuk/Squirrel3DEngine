#pragma once

#include "Font.h"
#include "Element.h"
#include "Panel.h"
#include "Menu.h"
#include "MenuContentSource.h"
#include <Common/Input.h>

namespace Squirrel {
namespace GUI { 

class SQGUI_API Manager
{
public:

	enum State
	{
		stateIdle = 0,
		stateProcessInput,
		stateRender
	};

private:

	static const int mDefaultMenuWidth = 160;

	GUI::Font * mMainFont;

	State mState;

	bool mEatInput;

	Panel * mMainPanel;
	Menu * mRootMenu;

	Element * mDraggingElem;
	Element * mLMBClicked;

private:
	Manager();

public:

	~Manager();

	static Manager& Instance();

	State getState() { return mState; }

	void init();

	//static void saveElement(Element *elem, std::string &filename);
	//static void loadElement(Element *elem, std::string &filename);
	//static Element *loadElement(std::string &filename);

	void showMenu(MenuContentSource * menuSrc, tuple2i pos);
	void showMenu(MenuContentSource * menuSrc);

	Menu& getRootMenu() { return *mRootMenu; }
	Panel& getMainPanel() { return *mMainPanel; }
	void render();
	void update();

	GUI::Font * getMainFont() { return mMainFont; }

private:

	void onLU();
	bool onLD();
	void onRU();
	void onRD();
	void onMM();
	void onCh(char_t ch);
	void onKD(Input::NamedKey key);
	void onKU(Input::NamedKey key);
	bool onMW(float delta);
	void onDBL();

	void initObjectTypes();
};

}//namespace GUI { 
}//namespace Squirrel {