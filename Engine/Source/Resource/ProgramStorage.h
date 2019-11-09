#pragma once

#include "Program.h"
#include "ResourceStorage.h"

namespace Squirrel {

namespace Resource { 

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API ResourceStorage<Program>;
#endif
	
class SQRESOURCE_API ProgramStorage: 
	public ResourceStorage<Program>,
	public Program::SourceLoader
{
	static ProgramStorage * sActiveLibrary;

public:
	ProgramStorage();
	virtual ~ProgramStorage();

	void setAsActive();
	static ProgramStorage * Active();
	
	std::string loadSource(const char_t * sourceFile, StoredObject * relativeTo = NULL);

	void checkModifications();

protected:
	virtual Program* load(Data * data);
};


}//namespace Resource { 

}//namespace Squirrel {