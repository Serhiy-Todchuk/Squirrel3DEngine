// Settings.cpp: implementation of the Settings class.
//
//////////////////////////////////////////////////////////////////////

#include "Settings.h"
#include "Data.h"
#include "StringUtils.h"
#include "macros.h"

#ifdef	_WIN32
# pragma	warning (disable:4996)
#endif

namespace Squirrel {

Settings * Settings::sDefault = NULL;

Settings * Settings::Default()
{
	return sDefault;
}

Settings::Settings(const char_t * fileName)
{
	mFileName = fileName;
	Data * file = new Data(fileName);
	if(file)
	{
		parseFile(file);
		delete file;
	}
}

Settings::~Settings()
{
	if(this == sDefault)
	{
		sDefault = NULL;
	}
}

void Settings::setAsDefault()
{
	sDefault = this;
}

void Settings::writeToFile()
{
	Data *file = new Data(NULL, (size_t)0);
	buildFile(file);
	file->writeToFile(mFileName.c_str());
	delete file;
}

void Settings::parseFile(Data * file)
{
	SECTION * currentSection = NULL;

	const char_t nextLineTerm = '\n';
	const char_t sectionStartChar = '[';
	//const char_t sectionEndChar = ']';
	const char_t assignmentChar = '=';

	std::string line;

	while(file->getPos() < file->getLength())
	{
		//get line
		if(!file->readString(line, nextLineTerm)) break;
		trim(line);

		//skip empty line
		if(line.length() == 0) continue;

		//check if line is section
		if(line[0] == sectionStartChar)
		{
			//section

			std::string sectionName = line.substr(1, line.length() - 2);
			currentSection = getSection(sectionName);
		}
		else
		{
			//setting

			//get setting name
			std::string settingName;
			size_t assignPos = line.find_first_of(assignmentChar);
			if(assignPos == std::string::npos)
			{
				//wrong setting - skip it
				continue;
			}
			settingName = line.substr(0, assignPos - 1);
			trim(settingName);

			//get setting value
			std::string settingValue;
			settingValue = line.substr(assignPos + 1);
			trim(settingValue);

			//add setting to section
			if(currentSection == NULL)
			{
				currentSection = getSection("");
			}
			(*currentSection)[settingName] = settingValue;
		}
	}
}

void Settings::buildFile(Data * file)
{
	const char_t* nextLine = "\n";
	const char_t* sectionStart = "[";
	const char_t* sectionEnd = "]";
	const char_t* assignment = "\t= ";

	for(SECTIONS_MAP::iterator itSec = mSettings.begin(); itSec != mSettings.end(); ++itSec)
	{
		//write section name
		std::string sectionName( sectionStart );
		sectionName += (itSec->first + sectionEnd) + nextLine;
		file->putData( sectionName.c_str(), sectionName.length() );
		
		//write settings
		SECTION& section = itSec->second;
		for(SECTION::iterator it = section.begin(); it != section.end(); ++it)
		{
			std::string setting = (it->first + assignment) + it->second + nextLine;
			file->putData( setting.c_str(), setting.length() );
		}

		file->putData( nextLine, strlen(nextLine) );
	}
}

Settings::SECTION * Settings::getSection(const std::string& sectionName)
{
	SECTIONS_MAP::iterator it = mSettings.find(sectionName);
	if(it != mSettings.end())
	{
		return &it->second;
	}

	mSettings[sectionName] = SECTION();
	return &mSettings[sectionName];
}

Settings::SECTION * Settings::tryGetSection(const std::string& sectionName)
{
	SECTIONS_MAP::iterator it = mSettings.find(sectionName);
	if(it != mSettings.end())
	{
		return &it->second;
	}
	return NULL;
}

std::string	Settings::getString	(const char_t * sectionName, const char_t * name, const std::string& defaultValue)
{
	std::string value;
	SECTION * section = getSection(sectionName);
	SECTION::iterator it = section->find(name);
	if(it == section->end())
	{
		value = defaultValue;
		(*section)[name] = value;
	}
	else
	{
		value = it->second;
	}
	return value;
}

bool Settings::tryGetInt(const char_t * sectionName, const char_t * name, int &outValue)
{
	SECTION * section = tryGetSection(sectionName);

	if(section == NULL)
		return false;

	SECTION::iterator it = section->find(name);

	if(it == section->end())
		return false;

	outValue = atoi(it->second.c_str());
	return true;
}

int	Settings::getInt(const char_t * sectionName, const char_t * name, int defaultValue)
{
	SECTION * section = getSection(sectionName);
	SECTION::iterator it = section->find(name);
	if(it == section->end())
	{
		setInt(sectionName, name, defaultValue);
		return defaultValue;
	}
	return atoi(it->second.c_str());
}

float Settings::getFloat(const char_t * sectionName, const char_t * name, float defaultValue)
{
	SECTION * section = getSection(sectionName);
	SECTION::iterator it = section->find(name);
	if(it == section->end())
	{
		setFloat(sectionName, name, defaultValue);
		return defaultValue;
	}
	return (float)atof(it->second.c_str());
}


void Settings::setString(const char_t * sectionName, const char_t * name, const std::string& value)
{
	SECTION * section = getSection(sectionName);
	(*section)[name] = value;
}

void Settings::setInt(const char_t * sectionName, const char_t * name, int value)
{
	char_t buffer[64];
	sprintf(buffer, "%d", value);
	SECTION * section = getSection(sectionName);
	(*section)[name] = buffer;
}

void Settings::setFloat	(const char_t * sectionName, const char_t * name, float value)
{
	char_t buffer[64];
	sprintf(buffer, "%1.3f", value);
	SECTION * section = getSection(sectionName);
	(*section)[name] = buffer;
}


} //namespace Squirrel {