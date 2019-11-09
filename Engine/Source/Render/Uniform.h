#pragma once

#include <Math/mathTypes.h>
#include <Reflection/AtomicWrapper.h>
#include <Reflection/Object.h>
#include <Common/DataMap.h>
#include <Common/HashString.h>
#include "macros.h"

#ifdef SQ_CPP0X
# include <unordered_map>
#else
#include <map>
#endif

namespace Squirrel {

namespace Render { 

//#if defined(_DEBUG) && defined(_WIN32)
//std::map and std::unordered_map are slow in debug
//# define UniformMap Trie
//  typedef HashString UniformString;
//#else
//# ifdef SQ_CPP0X
//#  define UniformMap std::unordered_map
//   typedef std::string UniformString;
//# else
#  define UniformMap std::map
   typedef HashString UniformString;
//# endif
//#endif

class SQRENDER_API UniformReceiver
{
public:
	virtual ~UniformReceiver() {}

	virtual void uniformArray(const UniformString& un, int count, const int * u)		= 0;
	virtual void uniformArray(const UniformString& un, int count, const float * u)		= 0;
	virtual void uniformArray(const UniformString& un, int count, const Math::vec2 * u)	= 0;
	virtual void uniformArray(const UniformString& un, int count, const Math::vec3 * u)	= 0;
	virtual void uniformArray(const UniformString& un, int count, const Math::vec4 * u)	= 0;
	virtual void uniformArray(const UniformString& un, int count, const Math::mat3 * u)	= 0;
	virtual void uniformArray(const UniformString& un, int count, const Math::mat4 * u)	= 0;

	virtual bool receivesUniform(const UniformString& un) const	= 0;
};

class SQRENDER_API UniformValue
{
public:
	enum ValueQuantity
	{
		vqSingle	= 1,
		vqVec2		= 2,
		vqVec3		= 3,
		vqVec4		= 4,
		vqMat3		= 9,
		vqMat4		= 16,
	};

	UniformValue();
	~UniformValue();

	const UniformString& getName() const { return mName; }

	const Reflection::AtomicWrapper * getValue() const { return mValue; }

	ValueQuantity getValueQuantity() const { return mValueQuantity; }

	bool sameValues(const UniformValue& otherValue) const;

private:

	friend class UniformContainer;

	//name of uniform in shader
	UniformString mName;

	//value holder
	Reflection::AtomicWrapper * mValue;

	//quantity of data type of value (e.g. 16 for mat4, 3 for vec3...)
	ValueQuantity mValueQuantity;

	//determines if mValue should be destroyed in destructor
	bool mValueOwner;
};

class SQRENDER_API UniformContainer:
	public UniformReceiver
{
	typedef UniformMap<UniformString, UniformValue *> UNIFORM_VALUES_MAP;

	UNIFORM_VALUES_MAP mValues;

	UniformContainer(const UniformContainer&);
	const UniformContainer& operator=(const UniformContainer&);

public:
	UniformContainer();
	virtual ~UniformContainer();

	bool sameValues(const UniformContainer& otherContainer) const;

	bool fetchUniform(UniformReceiver * receiver, const UniformString& un);

	void fetchUniforms(UniformReceiver * receiver);

	size_t getValuesNum() const { return mValues.size(); }
	void clear();

	//implement UniformReceiver

	void uniformArray(const UniformString& un, int count, const int * u);
	void uniformArray(const UniformString& un, int count, const float * u);
	void uniformArray(const UniformString& un, int count, const Math::vec2 * u);
	void uniformArray(const UniformString& un, int count, const Math::vec3 * u);
	void uniformArray(const UniformString& un, int count, const Math::vec4 * u);
	void uniformArray(const UniformString& un, int count, const Math::mat3 * u);
	void uniformArray(const UniformString& un, int count, const Math::mat4 * u);

	//receives all uniforms
	virtual bool receivesUniform(const UniformString& un) const 	{ 	return true; 	}

private:

	bool fetchUniform(UniformReceiver * receiver, UniformValue * value);

	template <class TType>
	void store(const UniformString& uName, int count, const TType * values, UniformValue::ValueQuantity quantity)
	{
		UNIFORM_VALUES_MAP::iterator it = mValues.find(uName);

		UniformValue * uValue = NULL;

		if(it != mValues.end())
		{
			uValue = it->second;
		}
		else
		{
			uValue = new UniformValue();

			uValue->mValueOwner		= true;
			uValue->mValueQuantity	= quantity;
			uValue->mName			= uName;

			uValue->mValue			= new Reflection::AtomicWrapperImpl<TType>(NULL, count);

			mValues[uName] = uValue;
		}

		//set value data
		for(int i = 0; i < count; ++i)
		{
			uValue->mValue->setValue<TType>(i, values[i]);
		}
	}

};

} //namespace Render {

} //namespace Squirrel {
