#pragma once

#include <common/common.h>
#include <GUI/Panel.h>
#include <GUI/Label.h>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

class SQEDITOR_API ProgressPanel
{
public://ctor/dtor
	ProgressPanel();
	~ProgressPanel();

public://methods

	//main control
	void show(const char_t * title);
	void close();

	void setText(const char_t * text);

private://members

	//UI content
	GUI::Panel * mPanel;
	GUI::Label * mProgress;
	GUI::Label * mTitle;
};

}//namespace Editor { 
}//namespace Squirrel {
