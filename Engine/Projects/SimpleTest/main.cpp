#include "SimpleTestApp.h"

using namespace Squirrel;
using namespace Auxiliary;

int WINAPI WinMain(HINSTANCE hThisInst,HINSTANCE hPrev,LPSTR str,int nWinMode)
{
	//_CrtSetDbgFlag(5);

    Application * app = Application::Create();

    if(app)
    {
        if(app->init())
        {
            MSG msg;
            while(1)
            {
				bool exit = false;
                while( PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) )
                {
                    int msgRet = 0;
                    if( (msgRet = GetMessage(&msg,NULL,0,0)) > 0 )
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                    {
						exit = true;
						break;
                    }
                }
                
				if(exit)
					break;

                app->process();
            }
        }
        
        delete app;
    }

	return 0;
}