// Application.h: interface for the Application class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Common/DestructionPool.h>
#include <Common/Log.h>
#include <Common/Settings.h>
#include <Common/Window.h>
#include <Render/IRender.h>
#include <Audio/IAudio.h>

namespace Squirrel {

namespace Auxiliary {
    
#define AUDIO_SETTINGS_SECTION      "Audio"
#define AUDIO_MODULE_NAME_SETTING   "AudioModuleName"

#define GRAPHICS_SETTINGS_SECTION	"Graphics"
#define RENDER_MODULE_NAME_SETTING	"RenderModuleName"
#define WINDOW_WIDTH_SETTING		"WindowWidth"
#define WINDOW_HEIGHT_SETTING		"WindowHeight"
#define FULLSCREEN_SETTING			"Fullscreen"
#define DISPLAY_FREQUENCY_SETTING	"DisplayFrequency"
#define DISPLAY_COLORBITS_SETTING	"DisplayColorBits"
#define DEPTHBITS_SETTING			"DepthBits"
#define STENCILBITS_SETTING			"StencilBits"
#define AA_SAMPLES_SETTING			"AASamples"
#define SWAP_INTERVAL_SETTING		"SwapInterval"
	
#define DEFAULT_AUDIO_MODULE_NAME	"SqOpenAL"
#define DEFAULT_RENDER_MODULE_NAME	"SqOpenGL"
	
#ifndef SQ_DEFAULT_WINDOW_TITLE
#	define SQ_DEFAULT_WINDOW_TITLE	"Squirrel Engine App"
#endif
	
class Application
{
public:
    /** Constructor */
	Application();
    /** Destructor */
	virtual ~Application();
    
    virtual bool init();
	
    virtual void process() = 0;
	
    virtual void onBecomeActive() {};

	static Application *GetApplication();
    
	static Application * Create();

	Window * getWindow() { return mWindow; }
	
protected:
    
    /** Initialises log file at executable's (app bundle) parent folder. 
     @param fileName Name of log file. If it doesn't have extension, .log will be added to the end.
        If it is NULL, then executable (app bundle) name + .log will be used.
     @return pointer to initialised log object.
     */
    Log * initLog(const char_t * fileName = NULL);
    
    /** Creates settings object from file at executable's (app bundle) parent folder.
     @param fileName Name of log file. If it doesn't have extension, .log will be added to the end.
     If it is NULL, then executable (app bundle) name + .ini will be used.
     @return Settings instance.
     */
    Settings * initSettings(const char_t * fileName = NULL);
    
    /** Loads audio module and initializes audio system from it.
     @param moduleName Name of audio library module. 
        If it is NULL, the module name will be read from Settings object obtained with getSettings method: 
        section - AUDIO_SETTINGS_SECTION, parameter - AudioModule.
        If it doesn't contain extension of file, and it is failed to load module without extension,
        then extension will be added automatically (depending on platform) and module will be loaded once more
        For Windows it is dll file in the same folder as executable is.
        For Mac OS X it is dylib file in the app bundle libraries folder.
        For iOS this parameter is skipped and default audio system will be initialised (most likely OpenAL).
     @return Audio system.
     */
    Audio::IAudio * initAudio(const char_t * moduleName = NULL);
    
    /** Creates and initialises window.
     @param title Text which will be displayed on window. If NULL then SQ_DEFAULT_WINDOW_TITLE macro will be used.
     @param displaySettings Info about display settings applied to display in case of fullscreen. 
        If window is not fullscreen then only window height and width will be taken from displaySettings.
        If displaySettings is NULL, this info will be read from Settings object obtained with getSettings 
        method (from GRAPHICS_SETTINGS_SECTION section), fullScreen param will be also skipped and read from there:
        Width - window width;
        Height - window height;
        Fullscreen - determines whether window is fullscreen (1) or not (0);
        Frequency - display refresh frequency in case of fullscreen;
        ColorBits - color depth of display in case of fullscreen.
     @see initSettings
     @param fullScreen Determines if need to create fullscreen window. 
        In case of fullscreen display mode also will be changed. 
        If displaySettings param is NULL then this param is skipped.
     @return Window instance.
     */    
    Window * initWindow(const char_t * title = NULL, const DisplaySettings * displaySettings = NULL, bool fullScreen = false);
    
    /** Loads renderer module, creates rendering context for window previously created by initWindow 
        and creates rendering system object.
     @param moduleName Name of rendering library module.
        If it is NULL, the module name will be read from Settings object obtained with getSettings method:
        section - GRAPHICS_SETTINGS_SECTION, parameter - RenderModule.
        If it doesn't contain extension of file, and it is failed to load module without extension, 
        then extension will be added automatically (depending on platform) and module will be loaded once more
        For Windows it is dll file in the same folder as executable is.
        For Mac OS X it is dylib file in the app bundle libraries folder.
        For iOS this parameter is skipped and default audio system will be initialised (OpenGL).
     @param pixelFormat Pixel format description used for creating rendering context. If it is NULL, 
        pixel format description will be read from Settings object obtained with getSettings method (from [Graphics] section):
        DepthBits;
        StencilBits;
        Samples.
     @return Rendering system.
     */
    Render::IRender * initRender(const char_t * moduleName = NULL, PixelFormat * pixelFormat = NULL);
    
    /** Getter for mSettings. Also ensures that mSettings is initialised, so most likelly it never returns NULL.
     @return Returns mSettings.
     */
    Settings * getSettings();

protected:    
    
    Window * mWindow;
    
    Settings * mSettings;
    
    DestructionPool * mDestructionPool;
	
	static Application * sApplication;

	bool mSaveSettings;
};

}//namespace Squirrel {
}//namespace Auxiliary {
