// Edit.cpp: implementation of the Edit class.
//
//////////////////////////////////////////////////////////////////////

#include "Edit.h"
#include "Render.h"
#include "Cursor.h"
#include <Common/Input.h>
#include <Common/TimeCounter.h>

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Edit, GUIEdit);

const float		Edit::CURSOR_BLINK_PERIOD = 0.8f;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Edit::Edit()
{
	SQREFL_SET_CLASS(GUI::Edit);
	setSizeSep(32, 16);

	mCursorIsVisible	= true;
	mSelecting			= false;
	mLastBlink			= 0;
	mSelectionSize		= 0;
	mTextCursorPos		= 0;
	mTextStartPos		= 0;
}

Edit::~Edit()
{

}

void Edit::draw()
{
	if(getDrawStyle() == 0) return;

	tuple2i p = getGlobalPos();

	//draw frame
	Render::Instance().drawBackPlane(p, getSize());
	Render::Instance().drawBorder(p, getSize(), true);

	//setup new constraints
	clipContent();

	drawContent();

	//return parent's constraints
	unclipContent();
}

void Edit::drawContent()
{
	tuple2i p = getGlobalPos();

	//draw selection
	if(Element::GetFocus() == this && mSelectionSize != 0)
	{
		int selStart	= mTextCursorPos;
		int selEnd		= mTextCursorPos + mSelectionSize;
		if(mSelectionSize < 0)
		{
			std::swap(selStart, selEnd);
		}

		int selStartWidth =	getVisibleTextWidth(selStart);

		int selEndWidth =	getVisibleTextWidth(selEnd);

		tuple2i selPos = p + tuple2i(selStartWidth + Render::TEXT_OFFSET.x, 1);
		tuple2i selSize = tuple2i(selEndWidth - selStartWidth, getSize().y - 2);

		Render::Instance().drawSelectPlane(selPos, selSize);
	}

	//draw text
	const char * visibleText = &getText().c_str()[mTextStartPos];
	Render::Instance().drawText(p, visibleText, getFontSize());

	//update cursor blink
	mLastBlink += TimeCounter::Instance().getDeltaTime();
	if(mLastBlink >= CURSOR_BLINK_PERIOD)
	{
		mLastBlink = 0;
		mCursorIsVisible = !mCursorIsVisible;
	}

	//draw text cursor
	if(Element::GetFocus() == this && mSelectionSize == 0) 
	{
		if(mCursorIsVisible)
		{
			//visible text width untill text cursor pos
			int strWidth =	getVisibleTextWidth(mTextCursorPos);

			tuple2i cursorPos = p - Render::TEXT_OFFSET;

			cursorPos.x += strWidth;

			Render::Instance().drawText(cursorPos, "|", getFontSize());
		}
	}
}

Font * Edit::getFont()
{
	return Render::Instance().getFont( getFontSize() );
}

std::string Edit::getSelectedText() const
{
	if(mSelectionSize == 0)
	{
		return std::string();
	}

	int selStart = mTextCursorPos;
	if(mSelectionSize < 0) selStart += mSelectionSize;

	int selSize = abs(mSelectionSize);
	
	return getText().substr(selStart, selSize);
}

int Edit::getVisibleTextWidth(int textPos)
{
	int textLen		= (int)getText().length();
	const char * visibleText = &getText().c_str()[mTextStartPos];

	int visibleTextPos	= Math::clamp(textPos - mTextStartPos, 0, textLen);

	return	getFont()->getStrWidth(visibleText, visibleTextPos);
}

void Edit::updateTextStartPos(int textPos)
{
	if(textPos < mTextStartPos)
	{
		mTextStartPos = textPos;
	}
	else
	{
		//check if textPos is outside of edit's size
		int maxVisibleTextWidth = getSize().x - Render::TEXT_OFFSET.x - 1;
		int textPosVisibleWidth = getVisibleTextWidth(textPos);
		if(textPosVisibleWidth > maxVisibleTextWidth)
		{
			int widthOfStringNeedToSkip = textPosVisibleWidth - maxVisibleTextWidth;
			//cut first characters with width of widthOfStringNeedToSkip
			mTextStartPos += getFont()->getMaxFittingLength(getText().c_str(), widthOfStringNeedToSkip) + 1;
		}
	}
}

Element * Edit::onLU()
{
	mSelecting = false;
	return this;
}

Element * Edit::onRU()
{

	return this;
}

Element * Edit::onMM()
{
	tuple2i cursor = Cursor::Instance().getPos();
	if(isIn(cursor))
	{
		Cursor::Instance().setAppearance(Cursor::tText);
	}
	if(mSelecting)
	{
		int posUnderMouse = getMousePosition();
		setSelectionSize( posUnderMouse - mTextCursorPos );
		return this;
	}
	return NULL;
}

Element * Edit::onUnFocus()
{
	textEditEnded();
	mTextStartPos = 0;
	return Element::onUnFocus();
}

void Edit::resetState()
{
	Element::resetState();
	mSelecting = false;
}

void Edit::setTextCursorPos(int pos)
{
	mTextCursorPos = Math::clamp(pos, 0, (int)getText().length());
	mSelectionSize = 0;
	mCursorIsVisible = true;

	updateTextStartPos(mTextCursorPos);
}

void Edit::setSelectionSize(int sz)
{
	mSelectionSize = sz;
	if(mSelectionSize != 0)
	{
		int selTextPos = mTextCursorPos + mSelectionSize;
		if(selTextPos < 0) 
			mSelectionSize = -mTextCursorPos;
		if(selTextPos > (int)getText().length()) 
			mSelectionSize = (int)getText().length() - mTextCursorPos;

		updateTextStartPos(selTextPos);
	}
}

int Edit::getMousePosition()
{
	return getTextPosition( Cursor::Instance().getPos().x - getGlobalPos().x );
}

int Edit::getTextPosition(int x)
{
	int textLen		= (int)getText().length();

	//width (px) of invisible front part of text
	int frontStrWidth =	getFont()->getStrWidth(getText().c_str(), mTextStartPos);

	int bounds = x + frontStrWidth;

	int len = getFont()->getMaxFittingLength(getText().c_str(), bounds);

	return Math::clamp<int>(len, 0, textLen);
}

Element * Edit::onLD()
{
	if(Element::GetFocus()!=this) 
	{
		mTextBeforeEditing = getText();
		setState(Element::stateActive);
		setTextCursorPos( 0 );
		setSelectionSize( (int)getText().length() );
	} 
	else 
	{
		mSelecting = true;
		setTextCursorPos( getMousePosition() );
	}
	return this;
}

Element * Edit::onRD()
{

	return this;
}

Element * Edit::onCh(char_t ch)
{
	//char_t vkBack = VK_BACK;
	//char_t vkEnter = VK_RETURN;
	//if(ch!=vkBack && ch!=vkEnter)
	{
		if(mSelectionSize != 0)
		{
			removeSelected();
		}
		mCursorIsVisible = true;
		insertCharAtIndex( mTextCursorPos, ch );
		setTextCursorPos( mTextCursorPos + 1 );
	}
	return this;
}

void Edit::textEditEnded()
{
	if(mTextBeforeEditing != getText())
	{
		processAction(VALUE_CHANGED_ACTION, this);
	}
}

Element * Edit::onKD(int key)
{
	Input::NamedKey namedKey = (Input::NamedKey)key;
	if(namedKey == Input::Left)
	{
		if(	Input::Get()->isKeyPressed(Input::LShift) || 
			Input::Get()->isKeyPressed(Input::RShift) )
		{
			setSelectionSize(mSelectionSize - 1);
		}
		else
		{
			setTextCursorPos(mTextCursorPos - 1);
		}
	}
	else
	if(namedKey == Input::Right)
	{
		if(	Input::Get()->isKeyPressed(Input::LShift) || 
			Input::Get()->isKeyPressed(Input::RShift) )
		{
			setSelectionSize(mSelectionSize + 1);
		}
		else
		{
			setTextCursorPos(mTextCursorPos + 1);
		}
	}
	else
	if(namedKey == Input::Enter)
	{
		setState(Element::stateStd);
		if(getMaster()) getMaster()->setFocus();
	}
	else
	if(namedKey == Input::Backspace)
	{
		if(mSelectionSize != 0)
		{
			removeSelected();
		}
		else
		{
			removeCharsAtIndex(mTextCursorPos);
			setTextCursorPos(mTextCursorPos - 1);
		}
	}
	else if(namedKey == Input::Delete)
	{
		if(mSelectionSize != 0)
		{
			removeSelected();
		}
		else
		{
			removeCharsAtIndex(mTextCursorPos + 1);
		}
	}
	else if(namedKey == Input::KeyC && Input::Get()->isKeyPressed(Input::LControl))
	{
		Clipboard * clipboard = Squirrel::Render::IRender::GetActive()->getWindow()->getClipboard();

		std::string selectedText = getSelectedText();

		clipboard->setText(selectedText.c_str());
	}
	else if(namedKey == Input::KeyX && Input::Get()->isKeyPressed(Input::LControl))
	{
		Clipboard * clipboard = Squirrel::Render::IRender::GetActive()->getWindow()->getClipboard();

		std::string selectedText = getSelectedText();

		clipboard->setText(selectedText.c_str());

		removeSelected();
	}
	else if(namedKey == Input::KeyV && Input::Get()->isKeyPressed(Input::LControl))
	{
		Clipboard * clipboard = Squirrel::Render::IRender::GetActive()->getWindow()->getClipboard();

		if(clipboard->isTextAvailable())
		{
			std::string text = clipboard->getText();

			if(text.length() > 0)
			{
				removeSelected();

				std::string str = Element::getText();

				int index = mTextCursorPos;

				std::string str1 = str.substr(0, index);
				std::string str2 = str.substr(index,str.length()-index);
				str = str1 + text + str2;

				Element::setText(str);

				mSelectionSize = (int)text.length();
			}
		}
	}

	return this;
}

void Edit::setText(const std::string& text)
{
	//if(Element::GetFocus() == this)//wtf
	//	return;

	Element::setText(text);
}

void Edit::removeSelected()
{
	int selStart = mTextCursorPos;
	if(mSelectionSize < 0) selStart += mSelectionSize;
	selStart += 1;

	int selSize = abs(mSelectionSize);

	removeCharsAtIndex(selStart, selSize);

	mTextCursorPos = selStart - 1;
	mSelectionSize = 0;
}

void Edit::insertCharAtIndex(int index, char ch) {

	std::string str = Element::getText();

	std::string str1 = str.substr(0,index);
	std::string str2 = str.substr(index,str.length()-index);
	str = str1 + ch + str2;

	Element::setText(str);
}

void Edit::removeCharsAtIndex(int index, int charNum) {

	if(index == 0 || index > (int)getText().length()) return;

	std::string str = Element::getText();

	int end = index + charNum - 1;

	std::string str1 = str.substr(0, index - 1);
	std::string str2 = str.substr(end,str.length()-end);
	str = str1 + str2;

	Element::setText(str);
}

}//namespace GUI { 
}//namespace Squirrel {