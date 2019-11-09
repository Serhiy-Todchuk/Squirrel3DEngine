// Log.h: interface for the Log class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <map>
#include <string>
#include "types.h"
#include "macros.h"

namespace Squirrel {

class Data;

class SQCOMMON_API Settings  
{
private:

	typedef std::map<std::string, std::string> SECTION;
	typedef std::map<std::string, SECTION> SECTIONS_MAP;

public:
	Settings(const char_t * fileName);
	~Settings();

public:

	bool		tryGetInt		(const char_t * section, const char_t * name, int &outValue);

	std::string	getString	(const char_t * section, const char_t * name, const std::string& defaultValue = "");
	int			getInt		(const char_t * section, const char_t * name, int defaultValue = 0);
	float		getFloat	(const char_t * section, const char_t * name, float defaultValue = 0);

	void		setString	(const char_t * section, const char_t * name, const std::string& value);
	void		setInt		(const char_t * section, const char_t * name, int value);
	void		setFloat	(const char_t * section, const char_t * name, float value);

	void writeToFile();

	void setAsDefault();
	static Settings * Default();

private:

	void parseFile(Data * file);
	void buildFile(Data * file);

	SECTION * getSection(const std::string& sectionName);
	SECTION * tryGetSection(const std::string& sectionName);

private:

	SECTIONS_MAP	mSettings;
	std::string		mFileName;

	static Settings * sDefault;
};

} //namespace Squirrel {