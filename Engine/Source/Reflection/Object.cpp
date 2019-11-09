#include "Object.h"

namespace Squirrel {
namespace Reflection {

ObjectCreator::CREATORS_MAP& ObjectCreator::GetObjectCreators()
{
	static CREATORS_MAP objectCreators;
	return objectCreators;
}

ObjectCreator * ObjectCreator::GetCreator(std::string className)
{
	ObjectCreator::CREATORS_MAP::iterator it = ObjectCreator::GetObjectCreators().find(className);

	if(it == ObjectCreator::GetObjectCreators().end())
	{
		return NULL;
	}

	return it->second;
}

Object * Object::Instantiate(std::string className)
{
	ObjectCreator * creator = ObjectCreator::GetCreator( className );
	return (creator == NULL) ? NULL : creator->create();
}

Object * Object::Instantiate(Deserializer * deserializer)
{
	std::string name, className;
	deserializer->beginReadObject(className, name);

	//don't support dynamic creation of objects without creator
	ASSERT(ObjectCreator::GetCreator( className ) != NULL);

	//create obj
	Object * object = Instantiate( className );

	object->deserialize( deserializer );

	deserializer->endReadObject();

	return object;
}

Object::Object()
{
	mFieldIndex = 0;
}

Object::~Object()
{
	for(FIELDS_MAP::iterator it = mFields.begin(); it != mFields.end(); ++it)
	{
		if(it->second.memoryMaster)
			DELETE_PTR( it->second.object );
	}
}

Object::Field * Object::addField(const std::string& name, Serializable * object)
{
	FIELDS_MAP::iterator it = mFields.find(name);
	if(it != mFields.end() && it->second.memoryMaster)
		DELETE_PTR(it->second.object);
	Field field;
	field.name			= name;
	field.object		= object;
	field.changeFlag	= NULL;
	field.classTag		= getClassName();
	field.index			= mFieldIndex++;
	field.memoryMaster	= true;
	return &(mFields[ name ] = field);
}

const Object::Field * Object::getField(const std::string& name) const
{
	FIELDS_MAP::const_iterator it = mFields.find( name );
	return it == mFields.end() ? NULL : &it->second;
}

Object::Field * Object::getField(const std::string& name)
{
	FIELDS_MAP::iterator it = mFields.find( name );
	return it == mFields.end() ? NULL : &it->second;
}

Serializable * Object::getFieldObject(const std::string& name) const
{
	const Field * field = getField(name);
	return field == NULL ? NULL : field->object;
}

void Object::serialize(Serializer * serializer)
{
	serializer->willWriteObjects( mFields.size() );

	for(FIELDS_MAP::iterator it = mFields.begin(); it != mFields.end(); ++it)
	{
		Field& field = it->second;

		serializer->beginWriteObject( field.object->getClassName(), field.name );

		field.object->serialize( serializer );
		
		serializer->endWriteObject();
	}
}

void Object::deserialize(Deserializer * deserializer)
{
	int membersNum = deserializer->getObjectsNum();

	for(int i = 0; i < membersNum; ++i)
	{
		std::string name, className;
		deserializer->beginReadObject(className, name);

		Field * field = getField( name );
		if(field != NULL)
		{
			//check type
			const std::string& fieldClassName = field->object->getClassName();
			ASSERT(fieldClassName == className);
		}
		else
		{
			//don't support dynamic creation of objects without creator
			ASSERT(ObjectCreator::GetCreator( className ) != NULL);

			//create member
			Serializable * fieldObj = Instantiate( className );
			addField(name, fieldObj);
		}

		field->object->deserialize( deserializer );
		if(field->changeFlag != NULL)
		{
			*field->changeFlag = true;
		}

		deserializer->endReadObject();
	}
}


}//namespace Reflection {
}//namespace Squirrel