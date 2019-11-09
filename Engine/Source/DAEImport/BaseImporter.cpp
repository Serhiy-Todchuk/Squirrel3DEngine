#include "BaseImporter.h"
#include <Common/Settings.h>

namespace Squirrel {
namespace DAEImport { 

const String BaseImporter::ID_ATTR				= "id"; 
const String BaseImporter::SUB_ID_ATTR			= "sid"; 
const String BaseImporter::URL_ATTR				= "url"; 
const String BaseImporter::NAME_ATTR			= "name"; 
const String BaseImporter::TYPE_ATTR			= "type"; 
const String BaseImporter::COUNT_ATTR			= "count"; 
const String BaseImporter::SOURCE_ATTR			= "source"; 
const String BaseImporter::SEMANTIC_ATTR		= "semantic"; 
const String BaseImporter::STRIDE_ATTR			= "stride"; 
const String BaseImporter::TARGET_ATTR			= "target"; 
const String BaseImporter::OFFSET_ATTR			= "offset"; 
const String BaseImporter::SET_ATTR				= "set"; 

const String BaseImporter::SOURCE_TAG			= "source";
const String BaseImporter::INPUT_TAG			= "input";
const String BaseImporter::FLOAT_ARRAY_TAG		= "float_array";
const String BaseImporter::NAME_ARRAY_TAG		= "Name_array";
const String BaseImporter::TECHNIQUE_COMMON_TAG	= "technique_common";
const String BaseImporter::ACCESSOR_TAG			= "accessor";

float	BaseImporter::mMetersInUnit		= 1.0f;
bool	BaseImporter::mZIsUp			= false;

BaseImporter::SourceNode * BaseImporter::readSourceNode(pugi::xml_node node)
{
	//!!! consider that sources are always before vertices, polylist and triangles
	if(SOURCE_TAG != node.name()) return NULL;

	SourceNode * data = new SourceNode();

	data->id = node.attribute(ID_ATTR.c_str()).value();

	int arraySize = 0;

	//read float values array
	
	pugi::xml_node floatArrayNode = node.child(FLOAT_ARRAY_TAG.c_str());

	if(floatArrayNode)
	{
		arraySize = floatArrayNode.attribute(COUNT_ATTR.c_str()).as_int();

		const char * floatArrayValues = floatArrayNode.child_value();

		data->floatArray = new FloatArray(arraySize, floatArrayValues);
	}

	//read string values array
	
	pugi::xml_node namesArrayNode = node.child(NAME_ARRAY_TAG.c_str());

	if(namesArrayNode)
	{
		arraySize = namesArrayNode.attribute(COUNT_ATTR.c_str()).as_int();

		const char * namesArrayValues = namesArrayNode.child_value();

		data->namesArray = new NameArray(arraySize, namesArrayValues);
	}

	//read accessor data

	pugi::xml_node techniqueNode = node.child(TECHNIQUE_COMMON_TAG.c_str());
	pugi::xml_node accessorNode = techniqueNode.child(ACCESSOR_TAG.c_str());

	int tuplesCount = accessorNode.attribute(COUNT_ATTR.c_str()).as_int();
	int arrayStride = accessorNode.attribute(STRIDE_ATTR.c_str()).as_int();

	if(arrayStride == 0)
	{
		arrayStride = 1;
	}

	if(tuplesCount == 0)
	{
		tuplesCount = arraySize / arrayStride;
	}

	if(data->floatArray != NULL)
		data->floatArray->setAccessorInfo(tuplesCount, arrayStride);

	return data;
}

BaseImporter::BaseImporter(void)
{
}

BaseImporter::~BaseImporter(void)
{
}

Math::mat4 BaseImporter::convertMatrix(Math::mat4 dst)
{

	/*
	// Convert to Y_UP, if different orientation
	if( parser.mUpDirection == ColladaParser::UP_X)
		pScene->mRootNode->mTransformation *= aiMatrix4x4( 
			 0, -1,  0,  0, 
			 1,  0,  0,  0,
			 0,  0,  1,  0,
			 0,  0,  0,  1);
	else if( parser.mUpDirection == ColladaParser::UP_Z)
		pScene->mRootNode->mTransformation *= aiMatrix4x4( 
			 1,  0,  0,  0, 
			 0,  0,  1,  0,
			 0, -1,  0,  0,
			 0,  0,  0,  1);
	*/
	/*
	if(parentLink == NULL && BaseImporter::mZIsUp)
	{
		Math::mat4 rot = Math::mat4::RotAxis(Math::vec3(1,0,0), -HALF_PI);
		modelNode->mTransform = rot * modelNode->mTransform;
	}
	*/
	Math::mat4 result = dst;
	if( mZIsUp )
	{
		Math::mat4 swapZ2YMat = swapZ2YMatrix();
		result = swapZ2YMat * dst;
	}
	float unitsInMeterInternal = Settings::Default()->getFloat("World", "UnitsInMeter", 10.0f);
	float scale = mMetersInUnit * unitsInMeterInternal;
	Math::mat4 scaleMatrix = Math::mat4::Scale(Math::vec3(scale, scale, scale));

	return scaleMatrix * result;
}

Math::vec3 BaseImporter::convertPosition(Math::vec3 dst)
{
	Math::vec3 result = mZIsUp ? Math::vec3(dst.x, dst.z, -dst.y) : dst;
	result *= mMetersInUnit;
	return result;
}

Math::vec3 BaseImporter::convertDirection(Math::vec3 dst)
{
	Math::vec3 result = mZIsUp ? Math::vec3(dst.x, dst.z, -dst.y) : dst;
	return result;
}

Math::mat4 BaseImporter::swapZ2YMatrix()
{
	return Math::mat4(	1.0f,	0.0f,	0.0f,	0.0f,
						0.0f,	0.0f,	1.0f,	0.0f,
						0.0f,	-1.0f,	0.0f,	0.0f,
						0.0f,	0.0f,	0.0f,	1.0f);
}

}//namespace DAEImport { 
}//namespace Squirrel {