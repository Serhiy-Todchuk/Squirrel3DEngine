#include "Uniform.h"

namespace Squirrel {

namespace Render {

using namespace Reflection;

UniformValue::UniformValue(): mValue(NULL), mValueQuantity(vqSingle), mValueOwner(false)
{
}
	
UniformValue::~UniformValue()
{
	if(mValueOwner)
	{
		DELETE_PTR(mValue);
	}
}

bool UniformValue::sameValues(const UniformValue& otherValue) const
{
	if(mValueQuantity != otherValue.mValueQuantity)
		return false;

	if(!mValue->sameValues(otherValue.mValue))
		return false;

	return true;
}

/*
void UniformValue::deserialize(Deserializer * deserializer)
{
	Object::deserialize(deserializer);
	DATA_PTR data = deserializer->readObjectData();

	std::list<std::string> values;
	split(std::string(data->data), " \t\n", values);

	if(values.size() == 0)
		return;

	mValue		= new AtomicWrapperImpl<float>(NULL, values.size());
	mValueOwner	= true;

	size_t i = 0;
	FOREACH(std::list<std::string>::iterator, it, values)
	{
		mValue->setValue<float>(i++, static_cast<float>(atof((*it).c_str())) );
	}
}

void UniformValue::serialize(Serializer * serializer)
{
	Object::serialize(serializer);

	std::stringstream strStr;
	for(int i = 0; i < mValue->getValuesNum(); ++i)
	{
		strStr << mValue->getValue<float>(i);
		if(i < mValue->getValuesNum() - 1)
			strStr << " ";
	}

	std::string str = strStr.str();
	serializer->writeObjectData(str.c_str(), str.length());
}
*/
UniformContainer::UniformContainer()
{
}

UniformContainer::~UniformContainer()
{
	clear();
}

void UniformContainer::clear()
{
	FOREACH(UNIFORM_VALUES_MAP::iterator, it, mValues)
	{
		DELETE_PTR(it->second);
	}
	mValues.clear();
}

bool UniformContainer::sameValues(const UniformContainer& otherContainer) const
{
	if(otherContainer.mValues.size() != mValues.size())
		return false;
	
	FOREACH(UNIFORM_VALUES_MAP::const_iterator, itValue, mValues)
	{
		UNIFORM_VALUES_MAP::const_iterator itOtherValue = otherContainer.mValues.find( itValue->first );

		if(itOtherValue == otherContainer.mValues.end())
			return false;

		if(itValue->second->sameValues( *itOtherValue->second ))
			return false;
	}

	return true;
}
	
void UniformContainer::fetchUniforms(UniformReceiver * receiver)
{
	for(UNIFORM_VALUES_MAP::const_iterator it = mValues.begin(); it != mValues.end(); ++it)
	{
		fetchUniform(receiver, it->second);
	}
}

bool UniformContainer::fetchUniform(UniformReceiver * receiver, const UniformString& uName)
{
	UNIFORM_VALUES_MAP::iterator it = mValues.find(uName);

	if(it == mValues.end())
		return false;

	return fetchUniform(receiver, it->second);
}

bool UniformContainer::fetchUniform(UniformReceiver * receiver, UniformValue * value)
{
	AtomicWrapper * atom = value->mValue;

	if(atom->getType() == AtomicWrapper::tInt32)
	{
		//now supported only quantity UniformValue::vqSingle
		receiver->uniformArray(value->mName, atom->getValuesNum(), &atom->getValue<int>());
	}
	if(atom->getType() == AtomicWrapper::tFloat32)
	{
		switch(value->mValueQuantity)
		{
		case UniformValue::vqSingle:
			receiver->uniformArray(value->mName, atom->getValuesNum(), &atom->getValue<float>());
			break;
		case UniformValue::vqVec2:
			receiver->uniformArray(value->mName, atom->getValuesNum() / 2, reinterpret_cast<const Math::vec2*>(&atom->getValue<float>()));
			break;
		case UniformValue::vqVec3:
			receiver->uniformArray(value->mName, atom->getValuesNum() / 3, reinterpret_cast<const Math::vec3*>(&atom->getValue<float>()));
			break;
		case UniformValue::vqVec4:
			receiver->uniformArray(value->mName, atom->getValuesNum() / 4, reinterpret_cast<const Math::vec4*>(&atom->getValue<float>()));
			break;
		case UniformValue::vqMat3:
			receiver->uniformArray(value->mName, atom->getValuesNum() / 9, reinterpret_cast<const Math::mat3*>(&atom->getValue<float>()));
			break;
		case UniformValue::vqMat4:
			receiver->uniformArray(value->mName, atom->getValuesNum() / 16, reinterpret_cast<const Math::mat4*>(&atom->getValue<float>()));
			break;
		default:
			ASSERT(false);//not implemented
			break;
		}
	}

	return true;
}

void UniformContainer::uniformArray(const UniformString& un, int count, const int * u)
{
	store<int>(un, count, u, UniformValue::vqSingle);
}

void UniformContainer::uniformArray(const UniformString& un, int count, const float * u)
{
	store<float>(un, count, u, UniformValue::vqSingle);
}

void UniformContainer::uniformArray(const UniformString& un, int count, const Math::vec3 * u)
{
	store<float>(un, count * 3, &u->x, UniformValue::vqVec3);
}

void UniformContainer::uniformArray(const UniformString& un, int count, const Math::vec2 * u)
{
	store<float>(un, count * 2, &u->x, UniformValue::vqVec2);
}

void UniformContainer::uniformArray(const UniformString& un, int count, const Math::vec4 * u)
{
	store<float>(un, count * 4, &u->x, UniformValue::vqVec4);
}

void UniformContainer::uniformArray(const UniformString& un, int count, const Math::mat3 * u)
{
	store<float>(un, count * 9, &u->x.x, UniformValue::vqMat3);
}

void UniformContainer::uniformArray(const UniformString& un, int count, const Math::mat4 * u)
{
	store<float>(un, count * 16, &u->x.x, UniformValue::vqMat4);
}

} //namespace Render {

} //namespace Squirrel {
