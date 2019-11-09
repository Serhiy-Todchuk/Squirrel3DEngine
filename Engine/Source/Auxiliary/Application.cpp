// Application.cpp: implementation of the Application class.
//
//////////////////////////////////////////////////////////////////////

#include "Application.h"
#include <Common/DynamicLibrary.h>
#include <Common/Platform.h>
#include <FileSystem/Path.h>
#include <FileSystem/FileStorage.h>
#ifdef _WIN32
#include <Common/Windows/WindowsWindowManager.h>
#endif

namespace Squirrel {
namespace Auxiliary {

DynamicLibrary * LoadModule(const char_t * name)
{
    std::string moduleName = name;
#if defined(_WIN32) && defined(_DEBUG)
	moduleName += "D";
#endif
	
	DynamicLibrary * module = new DynamicLibrary(moduleName.c_str());
    
    if(!module->isOpened())
    {
		moduleName += ".";
		moduleName += DynamicLibrary::GetDLExtension();
        module->open(moduleName.c_str());
    }
    
    return module;
}
	
std::string CombineAppFileName(const char_t * fileName, const char_t * extension)
{
	std::string newFileName;
	
	if(fileName == NULL)
	{
		newFileName = Platform::GetAppName();
		newFileName = FileSystem::Path::GetFileNameWithoutExtension(newFileName);
	}
	else
	{
		newFileName = fileName;
	}
	
	//if has no extension then add it
	if(newFileName.find(".") == std::string::npos)
		newFileName += extension;

	//apply root folder
	newFileName = FileSystem::Path::Combine(Platform::GetRootPath(), newFileName);
	
	return newFileName;
}
	
Application * Application::sApplication = NULL;
	
Application * Application::GetApplication()
{
	return sApplication;
}
	
Application::Application():
    mWindow(NULL), mSettings(NULL), mSaveSettings(false)
{
	mDestructionPool = new DestructionPool;
	
	if(sApplication == NULL)
		sApplication = this;
	else
		ASSERT(false);//warn about more than one instance of app
}

Application::~Application()
{
	if(mSaveSettings)
		Settings::Default()->writeToFile();

	DELETE_PTR(mDestructionPool);
	Log::Instance().finish();
}

bool Application::init()
{
    Log * log = initLog();
	
	if(!initAudio() && log)
	{
		log->error("Application::init", "Failed to initialise audio system!");
	}
	
	bool renderSuccess = initRender() != NULL;//calls also initWindow
	
	if(!renderSuccess && log)
	{
		log->error("Application::init", "Failed to initialise rendering system!");
	}
	
	//count only success of render itialization as it is the most important
	
	return renderSuccess;
}

Settings * Application::getSettings()
{
    if(mSettings != NULL)
        return mSettings;
    
    mSettings = Settings::Default();
    
    if(mSettings != NULL)
        return mSettings;
    
    return initSettings();
}
	
Log * Application::initLog(const char_t * fileName)
{
	std::string logFileName = CombineAppFileName(fileName, ".log");
    
	Log::Instance().init(logFileName.c_str(), Log::sevInformation);
    
    return &Log::Instance();
}

Settings * Application::initSettings(const char_t * fileName)
{
	std::string settingsFileName = CombineAppFileName(fileName, ".ini");

	mSaveSettings = !FileSystem::FileStorage::IsFileExist(settingsFileName.c_str());

	mSettings = new Settings(settingsFileName.c_str());
	
    mDestructionPool->addObject(mSettings);
    
    return mSettings;
}

Audio::IAudio * Application::initAudio(const char_t * moduleName)
{
	std::string audioModuleName;
    
    if(moduleName == NULL)
    {
        audioModuleName = getSettings()->getString(AUDIO_SETTINGS_SECTION, AUDIO_MODULE_NAME_SETTING, DEFAULT_AUDIO_MODULE_NAME);
    }
    else
    {
        audioModuleName = moduleName;
    }
	
	DynamicLibrary * audioDL = LoadModule(audioModuleName.c_str());
	
	if(audioDL == NULL)
		return NULL;
	
	mDestructionPool->addObject(audioDL);
    
	typedef Audio::IAudio * (*CreateAudioFuncPtr)();
	typedef void (*DestroyAudioFuncPtr)(Audio::IAudio *);
    
	CreateAudioFuncPtr CreateAudio		= (CreateAudioFuncPtr)audioDL->getSymbol("CreateAudio");
	DestroyAudioFuncPtr DestroyAudio	= (DestroyAudioFuncPtr)audioDL->getSymbol("DestroyAudio");
    
	Audio::IAudio * audio = CreateAudio();
	if(!audio->init())
	{
		mDestructionPool->executeDestruction(audioDL);//just for showcase
		DestroyAudio(audio);
		return NULL;
	}
	audio->setAsActive();
    mDestructionPool->addObject(audio, DestroyAudio);
	
	return audio;
}

Window * Application::initWindow(const char_t * title, const DisplaySettings * displaySettings, bool fullScreen)
{
	WindowManager * wndManager = WindowManager::ActiveWindowManager();
    if(wndManager == NULL)
    {
        wndManager = Platform::CreateWindowManager();
#ifdef _WIN32
		static_cast<WindowsWindowManager *>(wndManager)->mApp = this;
#endif
    }
    mDestructionPool->addObject(wndManager);
	
	tuple2i screenSize = wndManager->getScreenResolution();
	
	DisplaySettings localDS;
	if(displaySettings == NULL)
	{
		fullScreen			= getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, FULLSCREEN_SETTING, 1) != 0;
		if(	!getSettings()->tryGetInt(GRAPHICS_SETTINGS_SECTION, WINDOW_WIDTH_SETTING, localDS.width) || 
			!getSettings()->tryGetInt(GRAPHICS_SETTINGS_SECTION, WINDOW_HEIGHT_SETTING, localDS.height))
		{
			localDS.width	= screenSize.x;
			localDS.height	= screenSize.y;
		}
		if(fullScreen)
		{
			localDS.freq		= getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, DISPLAY_FREQUENCY_SETTING, 60);
			localDS.colorBits	= getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, DISPLAY_COLORBITS_SETTING, 32);
		}
	}
	else
	{
		localDS = *displaySettings;
	}
        
	mWindow = wndManager->createWindow((title == NULL) ? SQ_DEFAULT_WINDOW_TITLE : title, localDS, fullScreen);
	mWindow->setMainWindow();
	
	return mWindow;
}

Render::IRender * Application::initRender(const char_t * moduleName, PixelFormat * pixelFormat)
{
	std::string renderModuleName;
    
    if(moduleName == NULL)
    {
        renderModuleName = getSettings()->getString(GRAPHICS_SETTINGS_SECTION, RENDER_MODULE_NAME_SETTING, DEFAULT_RENDER_MODULE_NAME);
    }
    else
    {
        renderModuleName = moduleName;
    }
	
	DynamicLibrary * renderDL = LoadModule(renderModuleName.c_str());
	
	if(renderDL == NULL)
		return NULL;
	
	mDestructionPool->addObject(renderDL);

	typedef Render::IRender * (*CreateRenderFuncPtr)();
	typedef void (*DestroyRenderFuncPtr)(Render::IRender *);
	typedef Context * (*CreateContextFuncPtr)(const PixelFormat&, Window *);
	typedef void (*DestroyContextFuncPtr)(Context *);
    
	CreateRenderFuncPtr CreateRender	= (CreateRenderFuncPtr)renderDL->getSymbol("CreateRender");
	DestroyRenderFuncPtr DestroyRender	= (DestroyRenderFuncPtr)renderDL->getSymbol("DestroyRender");
	CreateContextFuncPtr CreateContext	= (CreateContextFuncPtr)renderDL->getSymbol("CreateContext");
	DestroyContextFuncPtr DestroyContext= (DestroyContextFuncPtr)renderDL->getSymbol("DestroyContext");
    
	PixelFormat localPF;
	if(pixelFormat == NULL)
	{
		localPF.depthBits		= getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, DEPTHBITS_SETTING, 24);
		localPF.stencilBits		= getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, STENCILBITS_SETTING, 8);
		localPF.samples			= getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, AA_SAMPLES_SETTING, 0);
	}
	else
	{
		localPF = *pixelFormat;
	}
    
	if(mWindow == NULL)
	{
		initWindow();
	}
	
	Context * context = CreateContext(localPF, mWindow);
	if(!context->isOk())
	{
		DestroyContext(context);
		return NULL;
	}

	context->getCaps();
    mDestructionPool->addObject(context, DestroyContext);
    
	int swapInterval = getSettings()->getInt(GRAPHICS_SETTINGS_SECTION, SWAP_INTERVAL_SETTING, 1);
	context->setSwapInterval(swapInterval);
    
	Context * graphicsStreamingContext = context->createSibling();
	mDestructionPool->addObject(graphicsStreamingContext, DestroyContext);
	mWindow->registerContext( graphicsStreamingContext );
	
	//init render
	Render::IRender * render = CreateRender();
	render->setAsActive();
	render->setWindow(mWindow);
    mDestructionPool->addObject(render, DestroyRender);

	return render;
}

}//namespace Squirrel {
}//namespace Auxiliary {
