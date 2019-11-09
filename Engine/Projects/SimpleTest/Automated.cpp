// Automated.cpp: implementation of the Automated class.
//
//////////////////////////////////////////////////////////////////////

#include "Automated.h"

SQ_REGISTER_BEHAVIOUR_SEED(Automated, Automated);

Automated::Automated()
{
	SQREFL_SET_CLASS(Automated);

}

Automated::~Automated()
{
}

void Automated::awake()
{
	Actor::awake();
}

void Automated::update()
{
	Actor::update();
}