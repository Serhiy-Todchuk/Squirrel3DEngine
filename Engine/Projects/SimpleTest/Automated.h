// Automated.h: interface for the Automated class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Actor.h"
#include <Math/AABB.h>

using namespace Squirrel;

class Automated : public Actor
{

	Math::AABB mMovementLimits;

public:
	Automated();
	virtual ~Automated();

	void setMovementLimits(Math::AABB limits) { mMovementLimits = limits; }

protected:
	virtual void awake();
	virtual void update();

private:
};

