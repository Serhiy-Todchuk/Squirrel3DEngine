#pragma once

#include "macros.h"
#include <memory>

namespace Squirrel {
namespace Reflection {
	
struct RawData
{
	RawData(): data(NULL), length(NULL), memoryOwner(false) {}
	RawData(char * theData, size_t len): data(theData), length(len), memoryOwner(false) {}
	RawData(char * theData, size_t len, bool copy): data(theData), length(len), memoryOwner(copy) {
		if(copy) {
			data = new char[length];
			memcpy(data, theData, length);
		}
	}
	RawData(const RawData& other): data(other.data), length(other.length), memoryOwner(other.memoryOwner) {
		if(memoryOwner) {
			data = new char[length];
			memcpy(data, other.data, length);
		}
	}
	~RawData() {
		if(memoryOwner && data != NULL)
			delete[]data;
	}
	
	char *	data;
	size_t	length;
	bool	memoryOwner;
};
	
typedef std::shared_ptr<RawData> DATA_PTR;	

}//namespace Reflection {
}//namespace Squirrel