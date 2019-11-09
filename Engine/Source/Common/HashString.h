
#pragma once

#include <string>
#include "types.h"
#include "macros.h"

class HashString  
{
public:

	typedef uint32 HashType;

private:

	HashType		mHash;
	std::string		mString;

public:
	HashString(): mHash(0)	{}
	HashString(const std::string& str)	{
		setString(str);
	}
	HashString(const char_t* str)	{
		setString(str);
	}
	~HashString() {};

public:

	inline const std::string& str() const { return mString; }
	inline const char_t * c_str() const { return mString.c_str(); }

	inline size_t length() const { return mString.length(); }

	inline HashType getHash() const { return mHash; }

	inline void setString(const std::string& str) { 
		mString = str;
		computeHash();
	}

	inline void computeHash()
	{
		mHash = ComputeHash((const unsigned char *)mString.c_str());
	}

	inline const bool operator <(const HashString& other) const
	{
		if(mHash < other.mHash)	return true;
		if(mHash > other.mHash)	return false;
		return mString < other.mString;
	}

	inline const bool operator == (const HashString& other) const
	{
		if(mHash == other.mHash)
		{
			return mString == other.mString;
		}
		return false;
	}
	inline const bool operator != (const HashString& other) const
	{
		if(mHash != other.mHash)
		{
			return true;
		}
		return mString != other.mString;
	}

public:

	static HashType ComputeHash(const unsigned char *str)
    {
        HashType hash = 5381;
        int c;

        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
    }
};