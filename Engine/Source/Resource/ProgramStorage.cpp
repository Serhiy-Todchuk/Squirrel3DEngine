#include "ProgramStorage.h"
#include <Common/macros.h>
#include <Common/Data.h>
#include <Common/Log.h>

namespace Squirrel {

namespace Resource { 

ProgramStorage * ProgramStorage::sActiveLibrary = NULL;

ProgramStorage * ProgramStorage::Active()
{
	return sActiveLibrary;
}

ProgramStorage::ProgramStorage()
{
	
}

ProgramStorage::~ProgramStorage()
{
	if(this == sActiveLibrary)
	{
		sActiveLibrary = NULL;
	}
}

Program* ProgramStorage::load(Data * data)
{
	Program * program = new Program;

	ASSERT( data != NULL );
	ASSERT( data->getLength() > 0 );

	program->setSourceLoader(this);
	program->load(data);

	DELETE_PTR(data);

	return program;
}

void ProgramStorage::checkModifications()
{
	for(_ID i = 0; i < getSize(); ++i)
	{
		Program * prg = this->get(i);
		if(prg && !prg->isChanged())
		{
			time_t oldTime = prg->getTimestamp();
			time_t newTime = getTimestamp(prg->getName());
			if(newTime > oldTime)
			{
				Data * data = getResourceData(prg->getName());
				if(data)
				{
					prg->load(data);
					prg->setTimestamp(newTime);

					Log::Instance().stream("ProgramStorage::checkModifications") << "Updated shader source for program " << prg->getName() << "";
				}
			}
		}
	}
}

void ProgramStorage::setAsActive()
{
	sActiveLibrary = this;
}
	
std::string ProgramStorage::loadSource(const char_t * sourceFile, StoredObject * relativeTo)
{
	Data * data = getResourceData(sourceFile);
	
	if(data == NULL)
		return "";
	
	return std::string((const char_t *)data->getData(), data->getLength());
}

}//namespace Resource { 

}//namespace Squirrel {