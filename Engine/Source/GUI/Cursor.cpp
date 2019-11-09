// Cursor.cpp: implementation of the Cursor class.
//
//////////////////////////////////////////////////////////////////////

#include "Cursor.h"
#include "Render.h"
#ifdef _WIN32
# include <windows.h>
#endif

namespace Squirrel {
namespace GUI {

Cursor::Cursor()
{
	mState=1;
	mPos=tuple2i(1,1);
	mPrevPos=tuple2i(1,1);
	mScroll = false;
	//img=0;
}

Cursor::~Cursor()
{
	//if(img) delete img;
}

Cursor& Cursor::Instance()
{
	static Cursor instance;
	return instance;
}

void Cursor::draw()
{
	/*
	if(state)
		if(img)		
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Painter::DrawImage(tuple2i(pos.x,pos.y+img->getHeight()), 1, img);
			glDisable(GL_BLEND);
		}
		*/
}

void Cursor::changePos(tuple2i p)
{
	mPos = p;
	Squirrel::Render::IRender::GetActive()->getWindow()->setCursorPosition(p);
}

/*
FSDImage * Cursor::getImg()
{
	return img;
}
*/
void Cursor::destroy()
{
	//if(img) delete img;
}

void Cursor::setAppearance(AppearanceType appearance)
{
	mAppearance = appearance;

#ifdef _WIN32
	static HCURSOR cursors[tNUMBEROFTYPES] = { NULL };
	if(cursors[appearance] == NULL)
	{
		char_t * cursorResource = NULL;

		switch(appearance)
		{
		case tNormal:	cursorResource = IDC_ARROW;		break;
		case tWait:		cursorResource = IDC_WAIT;		break;
		case tText:		cursorResource = IDC_IBEAM;		break;
		case tMove:		cursorResource = IDC_CROSS;		break;
		case tSizeNWSE:	cursorResource = IDC_SIZENWSE;	break;
		case tSizeNESW:	cursorResource = IDC_SIZENESW;	break;
		case tSizeNS:	cursorResource = IDC_SIZENS;	break;
		case tSizeWE:	cursorResource = IDC_SIZEWE;	break;
		case tHelp:		cursorResource = IDC_HELP;		break;
		case tStop:		cursorResource = IDC_NO;		break;
		case tStarting:	cursorResource = IDC_APPSTARTING; break;
		case tLink:		cursorResource = IDC_HELP;		break;
		}
		/* //TODO:
		IDC_SIZEALL
		IDC_UPARROW
		*/

		cursors[appearance] = LoadCursor(NULL, cursorResource);
	}

	if(cursors[appearance] != NULL)
		SetCursor(cursors[appearance]);
#endif
}

/*
int Cursor::img_load(std::string fname)
{
	destroy();
	img = FSDImageFactory::getImage( fname, FSFileSystem::getFileSystemForID(FS_PATH_IU_IMAGES_ID) );
	return img!=0?1:0;
}
*/

void Cursor::setPos(tuple2i p)
{
	/*
	tuple2i newPos = p;
	if(mScroll)
	{
		tuple2i screen = Squirrel::Render::IRender::GetScreenSize();
		tuple2i center = tuple2i(screen.x/2, screen.y/2);
		tuple2i diff = mPos - p;
		for(int i = 0; i < 2; ++i)
		{
			if(p[i] <= 0 && p[i] < mPos[i] && mPos[i] <= center[i])
			{
				newPos[i] = screen[i];
			}
			if(p[i] >= screen[i])
			{
				if(p[i] > mPos[i] && mPos[i] > center[i])
				{
					newPos[i] = 0;
				}
			}
		}
		if(newPos != p)
		{
			changePos(newPos);
			mPrevPos = newPos + diff;
		}
	}
	if(newPos == p)
	{
		mPrevPos = mPos;
		mPos = p;
	}
	*/

	if(mScroll)
	{
		changePos(mPrevPos);
		mPos = p;
	}
	else
	{
		mPrevPos = mPos;
		mPos = p;
	}
}

}//namespace GUI { 
}//namespace Squirrel {