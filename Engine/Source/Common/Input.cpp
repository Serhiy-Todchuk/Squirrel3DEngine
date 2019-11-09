#include "Input.h"

namespace Squirrel {

Input::Input() 
{
	update();
}

Input::~Input() 
{
}

void Input::resetMouse()
{

}

void Input::update()
{
	mInputString.clear();//kto ne uspel - tot opozdal :)

	mDownKeysQueue.clear();
	mUpKeysQueue.clear();

	mMouseHasBeenMoved	= false;
	mDoubleClicked		= false;
	mMouseScroll		= 0;
}

Input::KEYS_QUEUE::iterator Input::findKey(NamedKey key, KEYS_QUEUE * keysQueue)
{
	for(KEYS_QUEUE::iterator itKey = keysQueue->begin(); itKey != keysQueue->end(); ++itKey)
	{
		if((*itKey) == key) return itKey;
	}
	return keysQueue->end();
}

Input * Input::Get()
{
	static Input instance;
	return &instance;
}

void Input::pushDownKey( NamedKey key )	
{
	KEYS_QUEUE::iterator itPressedKey	= findKey( key, &mPressedKeysQueue);
	KEYS_QUEUE::iterator itDownKey		= findKey( key, &mDownKeysQueue);
	if(itPressedKey == mPressedKeysQueue.end() && itDownKey == mDownKeysQueue.end())
	{
		mDownKeysQueue.push_back( key ); 
		mPressedKeysQueue.push_back( key );
	}
}

void Input::pushUpKey( NamedKey key )
{
	KEYS_QUEUE::iterator itPressedKey = findKey( key, &mPressedKeysQueue);
	if(itPressedKey != mPressedKeysQueue.end())
	{
		mPressedKeysQueue.erase( itPressedKey );
	}
	mUpKeysQueue.push_back( key );
}

}//namespace Squirrel {
