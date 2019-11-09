#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 

//TODO: overload setText
class SQGUI_API Edit  : public Element
{
private:
	void removeCharsAtIndex(int index, int charNum = 1);
	void insertCharAtIndex(int index, char ch);

	void removeSelected();

	void setTextCursorPos(int pos);
	void setSelectionSize(int sz);

	void updateTextStartPos(int textPos);

	int getVisibleTextWidth(int textPos);

	int getMousePosition();
	int getTextPosition(int x);

	Font * getFont();

	static const float CURSOR_BLINK_PERIOD;

	float mLastBlink;
	bool mCursorIsVisible;
	bool mSelecting;
	int mTextCursorPos;
	int mSelectionSize;
	int mTextStartPos;

	std::string mTextBeforeEditing;

public:
	Edit();
	virtual ~Edit();

	virtual Element * onLU();
	virtual Element * onRU();
	virtual Element * onLD();
	virtual Element * onRD();
	virtual Element * onMM();
	virtual Element * onCh(char ch);
	virtual Element * onKD(int key);
	virtual void draw();
	virtual void resetState();
	virtual Element * onUnFocus();
	virtual void setText(const std::string& text);

	std::string getSelectedText() const;

protected:

	virtual void drawContent();
	virtual void textEditEnded();
};

}//namespace GUI { 
}//namespace Squirrel {