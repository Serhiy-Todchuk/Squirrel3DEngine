#include "../macros.h"
#include "../types.h"

namespace Squirrel {	

class MacUtils
{
public:
	static void * OpenAutoreleasePool();
	static void CloseAutoreleasePool(void * pool);
	
	static std::string GetBundleName();
	static std::string GetBundleParentFolder();

	static void TerminateProcess();
	
	static void DebugLog(const char_t * str);
};

}//namespace Squirrel {
