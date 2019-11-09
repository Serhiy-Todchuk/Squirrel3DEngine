#pragma once

#include "Serializable.h"
#include <sstream>

namespace Squirrel {
namespace Reflection {
	
class SQREFLECTION_API AtomicWrapper: public Serializable
{
public:

	typedef std::string::value_type TChar;

	enum EType
	{
		tUnknown = 0,
		
		//integer types
		tBool,
		tInt8,
		tUInt8,
		tInt16,
		tUInt16,
		tInt32,
		tUInt32,
		tInt64,
		tUInt64,
		
		//floating point types
		tFloat32,
		tFloat64,
		
		//string type
		tString
	};
	
protected:
	
	//static type mappper
	template <class TType> EType recognizeType		() { return tUnknown;	}
	//inverse static type mappper
	//NOTE: unknown type is not supported for this direction of recognition
	template <int Type> struct invTypeRecognizer		{ typedef void			type; };

protected://ctor
	AtomicWrapper();
public://dtor
	virtual ~AtomicWrapper();

	template <class TType>
	const TType& getValue(size_t index) const
	{
		//ASSERT( recognizeType<TType>() == mServingType );
		return reinterpret_cast<TType *>(mValues)[ index ];
	}

	template <class TType>
	const TType& getValue() const
	{
		return getValue<TType>(0);
	}

	template <class TType>
	void setValue(size_t index, const TType& value)
	{
		//ASSERT( recognizeType<TType>() == mServingType );
		reinterpret_cast<TType *>(mValues)[ index ] = value;
	}

	template <class TType>
	void setValue(const TType& value)
	{
		setValue<TType>(0, value);
	}

	inline size_t getValuesNum() const	{ return mValuesNum; }
	inline size_t getTypeSize() const	{ return mServingTypeSize; }
	inline EType getType() const	{ return mServingType; }
	inline std::string getTypeName() const	{ return mServingTypeName; }

	void* getBinaryBuffer(size_t index = 0) const;
	void setBinaryBufferData(void* data, size_t index = 0);
	size_t getBinaryBufferSize(size_t index = 0) const;

	virtual std::string getStringRepresentation(size_t index = 0) const = 0;
	virtual void parseString(const std::string& source, size_t index = 0) = 0;

	virtual void serialize(Serializer * serializer);
	virtual void deserialize(Deserializer * deserializer);

	bool sameValues(const AtomicWrapper* otherAtom) const;

protected://internal methods

	template <class TType>
	void setup(TType *values, size_t valuesNum = 1)
	{
		ASSERT( valuesNum >= 1 );
		
		mServingTypeName	= STRINGIFY( TType );
		mServingType		= recognizeType< TType >();

		mServingTypeSize	= sizeof( TType );

		mValuesNum			= valuesNum;

		if(values == NULL)
		{
			mValues				= new TType[ valuesNum ];
			mOwner				= true;
		}
		else
		{
			mValues				= values;
			mOwner				= false;
		}
	}

	void* getValuePointer(size_t index) const;

protected://members

	void *	mValues;
	size_t	mValuesNum;
	bool	mOwner;
	bool	mForceBinary;

	std::string	mServingTypeName;
	EType		mServingType;
	size_t		mServingTypeSize;
};
	
//static type mapppers
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<bool			>	() { return AtomicWrapper::tBool	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<int8			>	() { return AtomicWrapper::tInt8	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<uint8			>	() { return AtomicWrapper::tUInt8	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<int16			>	() { return AtomicWrapper::tInt16	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<uint16			>	() { return AtomicWrapper::tUInt16	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<int32			>	() { return AtomicWrapper::tInt32	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<uint32			>	() { return AtomicWrapper::tUInt32	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<int64			>	() { return AtomicWrapper::tInt64	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<uint64			>	() { return AtomicWrapper::tUInt64	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<float32		>	() { return AtomicWrapper::tFloat32	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<float64		>	() { return AtomicWrapper::tFloat64	;	}
template <>
inline AtomicWrapper::EType AtomicWrapper::recognizeType<std::string	>	() { return AtomicWrapper::tString	;	}

//inverse static type mapppers
//NOTE: unknown type is not supported for this direction of recognition
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tBool	>	{ typedef bool			type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tInt8	>	{ typedef int8			type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tUInt8	>	{ typedef uint8			type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tInt16	>	{ typedef int16			type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tUInt16	>	{ typedef uint16		type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tInt32	>	{ typedef int32			type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tUInt32	>	{ typedef uint32		type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tInt64	>	{ typedef int64			type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tUInt64	>	{ typedef uint64		type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tFloat32	>	{ typedef float32		type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tFloat64	>	{ typedef float64		type; };
template <> struct AtomicWrapper::invTypeRecognizer<AtomicWrapper::tString	>	{ typedef std::string	type; };


//AtomicWrapperImpl extends AtomicWrapper and always knows about actual holding type

template <class TType>
class AtomicWrapperImpl: public AtomicWrapper
{
	typedef TType TServingType;

public:
	AtomicWrapperImpl(TType *values, size_t valuesNum = 1)
	{
		setup<TType>( values, valuesNum );
	}
	virtual ~AtomicWrapperImpl() {};

	virtual std::string getStringRepresentation(size_t index = 0) const
	{
		ASSERT( index < mValuesNum );
		TType& value = static_cast<TType*>(mValues)[ index ];
		std::ostringstream stream;
		switch(mServingType)
		{
			case AtomicWrapper::tInt8:
			case AtomicWrapper::tUInt8:
				stream << (int32)value;
				break;
			default:
				stream << value;
				break;
		}
		return stream.str();
	}
	
	virtual void parseString(const std::string& source, size_t index = 0)
	{
		ASSERT( index < mValuesNum );
		TType& value = static_cast<TType*>(mValues)[ index ];
		std::istringstream stream( source );

		int32 intValue;
		switch(mServingType)
		{
			case AtomicWrapper::tInt8:
			case AtomicWrapper::tUInt8:
				stream >> intValue;
				value = intValue;
				break;
			default:
				stream >> value;
				break;
		}
	}
};

template <>
inline std::string AtomicWrapperImpl<std::string>::getStringRepresentation(size_t index) const
{
	ASSERT( index < mValuesNum );
	TServingType& value = static_cast<TServingType*>(mValues)[ index ];
	return value;
}

template <>
inline void AtomicWrapperImpl<std::string>::parseString(const std::string& source, size_t index)
{
	ASSERT( index < mValuesNum );
	TServingType& value = static_cast<TServingType*>(mValues)[ index ];
	value = source;
}


}//namespace Reflection {
}//namespace Squirrel