#pragma once

#include "macros.h"

namespace Squirrel {
namespace Render { 

class SQRENDER_API Utils
{
private://ctor/dtor
	Utils();
	~Utils();

public://methods

	static void Begin2D();
	static void End2D();
};

}//namespace Render { 
}//namespace Squirrel {
