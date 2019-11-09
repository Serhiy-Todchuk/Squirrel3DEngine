#pragma once

#include "../Math/mathTypes.h"
#include "types.h"
#include "tuple.h"

namespace Squirrel {

inline Math::vec4 colorBytesToVec4(tuple4ub color)
{
	return Math::vec4((float)color.x, (float)color.y, (float)color.z, (float)color.w) / 255.0f;
}

inline tuple4ub vec4ToColorBytes(Math::vec4 color)
{
	return tuple4ub((byte)(color.x * 255), (byte)(color.y * 255), (byte)(color.z * 255), (byte)(color.w * 255));
}

}//namespace Squirrel 