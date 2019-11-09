#pragma once

#include <map>
#include <Common/types.h>
#include <Render/IProgram.h>
#include "ResourceStorage.h"
#include "macros.h"

namespace Squirrel {

namespace Resource {
	
class SQRESOURCE_API Program:
	public StoredObject
{
public: //nested types

	class SourceLoader
	{
	public:
		SourceLoader() {}
		virtual ~SourceLoader() {}
		virtual std::string loadSource(const char_t * sourceFile, StoredObject * relativeTo = NULL) = 0;
	};
	
	typedef UniformMap<std::string, std::shared_ptr<Render::IProgram> > PROGRAMS_MAP;

private:

	PROGRAMS_MAP		mRenderPrograms;
	std::string			mShaderSource;

	SourceLoader *		mSourceLoader;
	
public:
	Program(void);
	virtual ~Program(void);

	void load(Data * data);
	
	void setSourceLoader(SourceLoader * sourceLoader) { mSourceLoader = sourceLoader; }

	Render::IProgram *	getRenderProgram(const std::string& params);

private:

	Render::IProgram * buildRenderProgram(const std::string& params);
	
	std::string resolveIncludes(std::string source);
	std::string resolveIncludeInLine(const std::string& line);
};


}//namespace Resource { 

}//namespace Squirrel {