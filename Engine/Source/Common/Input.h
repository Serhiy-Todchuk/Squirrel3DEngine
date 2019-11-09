#pragma once

#include "common.h"
#include <queue>

namespace Squirrel {

class SQCOMMON_API Input
{
public:
	enum NamedKey
	{
		Undefined = 0,
		Escape,
		Enter,
		Tab,
		CapsLock,
		NumLock,
		ScrollLock,
		Space,
		Backspace,
		Delete,
		Insert,
		Home,
		End,
		PageDown,
		PageUp,
		Pause,
		Print,
		Up,
		Down,
		Left,
		Right,
		LControl,
		LAlt,
		LShift,
		RControl,
		RAlt,
		RShift,

		Key0 = 0x30, //are the same as ASCII '0' - '9'
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,

		KeyA = 0x41, //are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,

		LOS,
		ROS,
		Apps,

		NumPlus,
		NumMinus,
		NumMult,
		NumSubt,
		NumPeriod,
		NumEquals,
		NumEnter,

		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,

		KeyMinus,
		KeyEqual,
		BackSlash,
		Slash,
		Tilda,
		
		Mouse0,
		Mouse1,
		Mouse2,
		Mouse3,
		Mouse4,
		Mouse5,
		Mouse6,
	};

	typedef std::deque<NamedKey> KEYS_QUEUE;

private:
	Input();
public:
	~Input();

	//update
	void pushDownKey( NamedKey key );
	void pushUpKey( NamedKey key );
	inline void		pushCharacter( std::string::value_type character )	{	mInputString.push_back( character );	}

	inline void		setMousePos(tuple2i pos)		{ mMousePos = pos; mMouseHasBeenMoved = true; }
	inline void		setMouseScroll(float scroll)	{ mMouseScroll = scroll; }

	inline void		setDoubleClicked(bool flag = true)				{ mDoubleClicked = flag; }

	void update();

	inline std::string	getInputString()			{	return mInputString;	}

	inline KEYS_QUEUE*	getUpKeysQueue()			{	return &mUpKeysQueue;	}
	inline KEYS_QUEUE*	getDownKeysQueue()			{	return &mDownKeysQueue;	}
	inline KEYS_QUEUE*	getPressedKeysQueue()		{	return &mPressedKeysQueue;	}

	inline bool			isKeyUp(NamedKey key)		{	return findKey(key, &mUpKeysQueue) != mUpKeysQueue.end();	}
	inline bool			isKeyDown(NamedKey key)		{	return findKey(key, &mDownKeysQueue) != mDownKeysQueue.end();	}
	inline bool			isKeyPressed(NamedKey key)	{	return findKey(key, &mPressedKeysQueue) != mPressedKeysQueue.end();	}

	inline tuple2i		getMousePos()				{ return mMousePos; }	
	inline float		getMouseScroll()			{ return mMouseScroll; }	

	inline bool			isMouseMoved()				{ return mMouseHasBeenMoved; }
	inline bool			isDoubleClicked()			{ return mDoubleClicked; }

	void resetMouse();

	static Input * Get();

private:

	KEYS_QUEUE::iterator findKey(NamedKey key, KEYS_QUEUE * keysQueue);

private:

	KEYS_QUEUE		mDownKeysQueue;
	KEYS_QUEUE		mUpKeysQueue;
	KEYS_QUEUE		mPressedKeysQueue;
	std::string		mInputString;

	tuple2i			mMousePos;

	float			mMouseScroll;

	bool			mMouseHasBeenMoved;

	bool			mDoubleClicked;
};

}//namespace Squirrel {