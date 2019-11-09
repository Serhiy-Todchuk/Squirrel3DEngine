// EngineApp.h: interface for the EngineApp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "EngineApp.h"
#include <Editor/Editor.h>

namespace Squirrel {

namespace Auxiliary {
	
class EditorApp:
	public EngineApp
{
public:	
    /** Constructor */
	EditorApp(): mEditor(NULL) {};
    /** Destructor */
	virtual ~EditorApp() {};
	
    virtual bool init()
    {
        bool success = EngineApp::init();
        
		mEditor = new Editor::Editor;
        mEditor->init(mWorld);
		
		mDestructionPool->addObject(mEditor);
		
        return success;
    }
	
	virtual void process()
	{
		mEditor->process();
		EngineApp::process();
	}
	    
protected:
	
	Editor::Editor * mEditor;
	
};

}//namespace Squirrel {
}//namespace Auxiliary {
