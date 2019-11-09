#pragma once

#include "../Image.h"
#include <Common/Data.h>

namespace Squirrel {

class Data;

namespace RenderData { 

class SQRENDER_API MacImageLoader
{
public:
	static Image * LoadImage(Data * fromData);
};

}//namespace RenderData { 

}//namespace Squirrel {

