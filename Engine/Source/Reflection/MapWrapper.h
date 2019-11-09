#pragma once

#include "Serializable.h"
#include "Object.h"
#include "AtomicWrapper.h"

namespace Squirrel {
namespace Reflection {

class SQREFLECTION_API MapWrapper: public Serializable
{
protected://ctor
	MapWrapper();
public://dtor
	virtual ~MapWrapper();

	virtual Object * getFirstValue() = 0;
	virtual Object * getNextValue() = 0;
	virtual size_t getSize() const = 0;

public://methods
};

template <class TMap, class TKey, class TValue>
class MapWrapperImpl: 
	public MapWrapper
{
	TMap * mMap;

	typename TMap::iterator mCurrentPos;

public://ctor/dtor
	MapWrapperImpl(TMap * map)
	{
		mMap = map;
	}
	~MapWrapperImpl()
	{

	}

	virtual Object * getFirstValue()
	{
		mCurrentPos = mMap->begin();
		return static_cast<Object *>(mCurrentPos->second);
	}

	virtual Object * getNextValue()
	{
		++mCurrentPos;
		if(mCurrentPos == mMap->end()) 
		{
			return NULL;
		}
		return static_cast<Object *>(mCurrentPos->second);
	}

	virtual size_t getSize() const
	{
		return mMap->size();
	}

	virtual void serialize(Serializer * serializer)
	{
		//write content of map

		serializer->willWriteObjects( mMap->size() );

		AtomicWrapperImpl<TKey> keyWrapper(NULL, 1);

		typename TMap::iterator it = mMap->begin();
		for(; it !=  mMap->end(); ++it)
		{
			keyWrapper.setValue(it->first);
			keyWrapper.serialize(serializer);

			serializer->beginWriteObject( it->second->getClassName(), it->second->getName() );
			it->second->serialize( serializer );
			serializer->endWriteObject();
		}
	}

	virtual void deserialize(Deserializer * deserializer)
	{
		int mapSize = deserializer->getObjectsNum();

		AtomicWrapperImpl<TKey> keyWrapper(NULL, 1);

		for(int i = 0; i < mapSize; ++i)
		{
			keyWrapper.deserialize(deserializer);
			const TKey& key = keyWrapper.template getValue<TKey>();

			std::string name, className;
			deserializer->beginReadObject(className, name);

			//don't support dynamic creation of objects without creator
			if(ObjectCreator::GetCreator( className ) != NULL)
			{
				//create object
				Object * obj = Object::Instantiate( className );
				if(obj != NULL)
				{
					obj->setName( name );
					obj->deserialize( deserializer );
					(*mMap)[key] = static_cast<TValue>(obj);
				}
			}
			else
			{
				//TODO: message
			}

			deserializer->endReadObject();
		}
	}

};

}//namespace Reflection {
}//namespace Squirrel