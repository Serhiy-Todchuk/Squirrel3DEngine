#pragma once
/*
#include "COLLADABUPlatform.h"
#include "COLLADAFWPrerequisites.h"
#include "COLLADAFWArrayPrimitiveType.h"
#include "COLLADAFWTypes.h"
#include "COLLADAFWFloatOrDoubleArray.h"
#include "COLLADAFWTypes.h"
*/
#include <Resource/ResourceManager.h>
#include <Math/mathTypes.h>
#include <pugixml/pugixml.hpp>
#include "Arrays.h"
#include "macros.h"

namespace Squirrel {
namespace DAEImport { 

typedef std::string UniqueId;
typedef std::string String;

class SQDAEIMPORTER_API BaseImporter
{
protected:

	struct SourceNode
	{
		SourceNode(): floatArray(NULL), namesArray(NULL) {}
		~SourceNode()
		{
			DELETE_PTR(floatArray);
			DELETE_PTR(namesArray);
		}
		FloatArray *	floatArray;
		NameArray *		namesArray;
		UniqueId		id;
		String			name;
		String			type;
	};

	static SourceNode * readSourceNode(pugi::xml_node node);

public:
	BaseImporter(void);
	virtual ~BaseImporter(void);

	//common XML tags
	static const String ID_ATTR; 
	static const String SUB_ID_ATTR; 
	static const String URL_ATTR; 
	static const String NAME_ATTR; 
	static const String TYPE_ATTR; 
	static const String COUNT_ATTR; 
	static const String SOURCE_ATTR; 
	static const String SEMANTIC_ATTR; 
	static const String STRIDE_ATTR; 
	static const String TARGET_ATTR; 
	static const String OFFSET_ATTR; 
	static const String SET_ATTR; 

	static const String SOURCE_TAG;
	static const String INPUT_TAG;
	static const String FLOAT_ARRAY_TAG;
	static const String NAME_ARRAY_TAG;
	static const String TECHNIQUE_COMMON_TAG;
	static const String ACCESSOR_TAG;

	static float	mMetersInUnit;
	static bool		mZIsUp;

public:

	//utils

	static Math::mat4 convertMatrix(Math::mat4 dst);
	static Math::vec3 convertPosition(Math::vec3 dst);
	static Math::vec3 convertDirection(Math::vec3 dst);
	static Math::mat4 swapZ2YMatrix();

	virtual void importObject(pugi::xml_node node) = 0;

protected:

	//UniqueId getUniqueId(pugi::xml_node node);

protected:

	pugi::xml_node mCurrentNode;
};

}//namespace DAEImport { 
}//namespace Squirrel {