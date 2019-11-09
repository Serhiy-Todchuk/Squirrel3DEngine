#include "Program.h"
#include <sstream>
#include <Common/macros.h>
#include <Common/Log.h>
#include <Render/IRender.h>

namespace Squirrel {

namespace Resource { 

using namespace Render;

//
//
//

Program::Program(): mSourceLoader(NULL)
{
}

Program::~Program(void)
{
}

IProgram * Program::buildRenderProgram(const std::string& params)
{
	std::string msg("Building program \"");
	msg += getName() + "\" with parameters \"";
	msg += params + "\"...";
	Log::Instance().report("Resource::Program::buildRenderProgram", msg.c_str(), Log::sevMessage);

	//create program
	IProgram * renderProgram = IRender::GetActive()->createProgram();

	renderProgram->setName(getName());
	renderProgram->setSource(mShaderSource);

	renderProgram->create(params);

	return renderProgram;
}

IProgram *	Program::getRenderProgram(const std::string& params)
{
	PROGRAMS_MAP::iterator it = mRenderPrograms.find(params);

	//found program
	if(it != mRenderPrograms.end())
	{
		return it->second.get();
	}

	//if not found then try to build it
	IProgram * renderProgram = buildRenderProgram(params);

	//add to map
	if(renderProgram)
	{
		mRenderPrograms[params] = std::shared_ptr<IProgram>(renderProgram);
	}

	return renderProgram;
}
	
std::string Program::resolveIncludeInLine(const std::string& inLine)
{
	std::string line = inLine;
	trim(line);
	
	if(line.length() == 0)
		return inLine;
	
	if(line[0] != '#')
		return inLine;
	
	line = line.substr(1);
	trim(line);
	
	if(line.find("include") != 0)
		return inLine;
	
	size_t openBracket = line.find_first_of('<');
	size_t closeBracket = line.find_last_of('>');
	
	bool relativeInclude = openBracket == std::string::npos;
	
	if(relativeInclude)
	{
		openBracket = line.find_first_of('"');
		closeBracket = line.find_last_of('"');
	}
	
	if(openBracket == std::string::npos || closeBracket == std::string::npos)
		return "";
	
	size_t length = closeBracket - openBracket - 1;
	std::string includeFile = line.substr(openBracket + 1, length);
	
	return mSourceLoader->loadSource(includeFile.c_str(), relativeInclude ? this : NULL);
}
	
std::string Program::resolveIncludes(std::string source)
{
	if(mSourceLoader == NULL)
		return source;
	
	std::stringstream output;
	
	const char_t * nextLine = "\n";
	
	std::list<std::string> lines;
	split(source, nextLine, lines);
	
	FOREACH(std::list<std::string>::iterator, itLine, lines)
	{
		output << resolveIncludeInLine(*itLine) << std::endl;
	}
	
	return output.str();
}

void Program::load(Data * data)
{
	mRenderPrograms.clear();
	mShaderSource = resolveIncludes(std::string((char_t *)data->getData(), data->getLength()));
}

}//namespace Resource { 

}//namespace Squirrel {