// SimpleTestApp.h: interface for the SimpleTestApp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Auxiliary/EditorApp.h>

using namespace Squirrel;
using namespace Auxiliary;

class SimpleTestApp:
	public EditorApp,
	public WindowDialogDelegate
{
public:	
	SimpleTestApp() {};
	virtual ~SimpleTestApp() {};
	
	///override EditorApp::process method
	virtual void process();
	    
protected:
	
	///implement WindowDialogDelegate interface to receive Exit dialog answer
	virtual void windowDialogEnd(int answer, void * contextInfo);
	
	///override EngineApp::initWorld method
    virtual void initWorld();
};