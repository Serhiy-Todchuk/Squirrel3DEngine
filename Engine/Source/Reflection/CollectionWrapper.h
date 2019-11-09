#pragma once

#include "Serializable.h"
#include "Object.h"

namespace Squirrel {
namespace Reflection {

class SQREFLECTION_API CollectionWrapper: public Serializable
{
protected://ctor
	CollectionWrapper();
public://dtor
	virtual ~CollectionWrapper();

	virtual Object * getFirst() = 0;
	virtual Object * getNext() = 0;
	virtual size_t getSize() const = 0;

public://methods
};

template <class TCollection, class TType>
class CollectionWrapperImpl: 
	public CollectionWrapper
{
	TCollection * mCollection;

	typename TCollection::iterator mCurrentPos;

public://ctor/dtor
	CollectionWrapperImpl(TCollection * collection)
	{
		mCollection = collection;
	}
	~CollectionWrapperImpl()
	{

	}

	virtual Object * getFirst()
	{
		mCurrentPos = mCollection->begin();
		return static_cast<Object *>(*mCurrentPos);
	}

	virtual Object * getNext()
	{
		mCurrentPos++;
		if(mCurrentPos == mCollection->end()) 
		{
			return NULL;
		}
		return static_cast<Object *>(*mCurrentPos);
	}

	virtual size_t getSize() const
	{
		return mCollection->size();
	}

	virtual void serialize(Serializer * serializer)
	{
		//write content of collection

		serializer->willWriteObjects( mCollection->size() );

		typename TCollection::iterator it = mCollection->begin();
		for(; it !=  mCollection->end(); ++it)
		{
			serializer->beginWriteObject( (*it)->getClassName(), (*it)->getName() );
			(*it)->serialize( serializer );
			serializer->endWriteObject();
		}
	}

	virtual void deserialize(Deserializer * deserializer)
	{
		int collectionSize = deserializer->getObjectsNum();

		for(int i = 0; i < collectionSize; ++i)
		{
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
					mCollection->push_back(static_cast<TType *>(obj));
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