#pragma once

#include "Serializable.h"
#include <memory>

namespace Squirrel {
namespace Reflection {

template <class TType>
class AtomicWrapperImpl;
	
template <class TCollection, class TType>
class CollectionWrapperImpl;
	
template <class TMap, class TKey, class TValue>
class MapWrapperImpl;

class SQREFLECTION_API Object:
	public Serializable
{
	//TODO: methods wrapping

public://nested types

	//TODO add support of functors with parameters and different return types

	class Functor
	{
	public:
		virtual ~Functor() {}
		virtual void execute() = 0;
	};

	template <class _T>
	class MethodWrapper: 
		public Functor 
	{
		_T * mObject;
		void (_T::*mMethod)();

	public:
		MethodWrapper(_T * obj, void (_T::*met)()):
			mObject(obj), mMethod(met) {}
		virtual ~MethodWrapper() {}

		// key method
		virtual void execute() {
			(*mObject.*mMethod)();
		}
	};

	struct Field
	{
		std::string			name;
		NAMES_LIST			attributes;
		Serializable *		object;
		bool *				changeFlag;
		int					index;
		std::string			classTag;
		bool				memoryMaster;
		std::shared_ptr<Functor> changeHandler;

		bool operator < (const Field& rhs) const	{
			return index < rhs.index;
		}

		bool hasAttribute(const std::string& name) const	{
			for(NAMES_LIST::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
				if((*it) == name)
					return true;
			return false;
		}

		bool hasAttributeContaining(const std::string& part, std::string& outAttrib) const	{
			for(NAMES_LIST::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
				if(it->find(part) != std::string::npos)
				{
					outAttrib = (*it);
					return true;
				}
			return false;
		}

		template <class _T>
		void setChangeHandler(_T * obj, void (_T::*met)())	{
			changeHandler.reset( new MethodWrapper<_T>(obj, met) );
		}
	};

	typedef std::map<std::string, Field>	FIELDS_MAP;

public://ctor/dtor

	Object();
	virtual ~Object();

public://methods

	const Field * getField(const std::string& name) const;
	Serializable * getFieldObject(const std::string& name) const;
	const FIELDS_MAP& getFields() const	{ return mFields; }

	const std::string&	getName	()	const		{return mName;}
	void	setName	(const std::string& name)	{mName	= name;}

	virtual void serialize(Serializer * serializer);
	virtual void deserialize(Deserializer * deserializer);

	static Object * Instantiate(std::string className);
	static Object * Instantiate(Deserializer * deserializer);

protected://internal methods

	Field * getField(const std::string& name);

	Field * addField(const std::string& name, Serializable * object);

	template <class TType>
	Field * wrapAtomicField(const std::string& name, TType * value, int valuesNum = 1)
	{
		return addField( name, new AtomicWrapperImpl<TType>( value, valuesNum ) );
	}

	//TODO: remake template arguments so compiler could automatically recognize TType from TCollection template
	template <class TCollection, class TType>
	Field * wrapCollectionField(const std::string& name, TCollection * collection)
	{
		return addField( name, new CollectionWrapperImpl<TCollection, TType>( collection ) );
	}

	//TODO: remake template arguments so compiler could automatically recognize TKey and TValue from TMap template
	template <class TMap, class TKey, class TValue>
	Field * wrapMapField(const std::string& name, TMap * map)
	{
		return addField( name, new MapWrapperImpl<TMap, TKey, TValue>( map ) );
	}

protected://members

	FIELDS_MAP		mFields;
	std::string		mName;
	int				mFieldIndex;
};

class SQREFLECTION_API ObjectCreator
{
	friend class Object;
	friend class ObjectCreatorFactory;

public://netsed types
	typedef std::map<std::string, ObjectCreator*> CREATORS_MAP;

protected://ctor
	ObjectCreator() {}

public://dtor
	virtual ~ObjectCreator() {}

public://methods
	virtual Object * create() = 0;

	inline const std::string& getClassName() const { return mClassName; }

	static ObjectCreator * GetCreator(std::string className);

protected://members
	std::string mClassName;

	static CREATORS_MAP& GetObjectCreators();
};

template <class TClass>
class ObjectCreatorImpl: 
	public ObjectCreator
{
public://ctor/dtor
	ObjectCreatorImpl() {}
	virtual ~ObjectCreatorImpl() {}

public://methods
	virtual Object * create()
	{
		return new TClass;
	}
};

class SQREFLECTION_API ObjectCreatorFactory
{
private://ctor
	ObjectCreatorFactory();

public://dtor
	~ObjectCreatorFactory();

public://methods

	template <class TClass>
	static ObjectCreator * Create(std::string className)
	{
		ObjectCreator::CREATORS_MAP::iterator it = ObjectCreator::GetObjectCreators().find( className );

		//if already existed
		if(it != ObjectCreator::GetObjectCreators().end())
		{
			return it->second;
		}

		//not created - create and store
		ObjectCreator * objCreator = new ObjectCreatorImpl<TClass>();
		objCreator->mClassName = className;
		ObjectCreator::GetObjectCreators()[ className ] = objCreator;
		return objCreator;
	}

};

template <class TClass>
struct ObjectCreatorHelper
{
	ObjectCreatorHelper(const char * className)
	{
		ObjectCreatorFactory::Create<TClass>(className);
	}
};

#define SQREFL_REGISTER_CLASS(TClass)							Squirrel::Reflection::ObjectCreatorHelper<TClass> TOKENPASTE2(_ObjectCreatorHelper, __COUNTER__)( #TClass );
#define SQREFL_REGISTER_CLASS_SEED(TClass, Seed)							Squirrel::Reflection::ObjectCreatorHelper<TClass> TOKENPASTE2(_ObjectCreatorHelper, Seed)( #TClass );
#define SQREFL_REGISTER_CLASS_NAMED(TClass, ClassName)			Squirrel::Reflection::ObjectCreatorHelper<TClass> TOKENPASTE2(_ObjectCreatorHelper, __COUNTER__)( ClassName );

#define SQREFL_SET_CLASS(TClass)								mClassNamesStack.push_back( #TClass );
#define SQREFL_SET_CLASS_NAMED(ClassName)						mClassNamesStack.push_back( ClassName );

#define SQREFL_WRAP_ATOMIC_FIELD(Field)							wrapAtomicField( #Field, &Field );
#define SQREFL_WRAP_ATOMIC_FIELD_ARR(Field, ValuesNum)			wrapAtomicField( #Field, &Field, ValuesNum );
#define SQREFL_WRAP_COLLECTION_FIELD(Field, TCollection, TElem)	wrapCollectionField<TCollection, TElem>( #Field, &Field );

}//namespace Reflection {
}//namespace Squirrel