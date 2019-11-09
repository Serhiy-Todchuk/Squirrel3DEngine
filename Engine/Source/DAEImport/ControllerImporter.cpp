#include "ControllerImporter.h"
#include "VisualSceneImporter.h"
#include <Resource/Skin.h>

namespace Squirrel {
namespace DAEImport { 
	
#define CONTROLLER_TAG			"controller"
#define SKIN_TAG				"skin"
#define BIND_SHAPE_MATRIX_TAG	"bind_shape_matrix"
#define JOINTS_TAG				"joints"
#define VERTEX_WEIGHTS_TAG		"vertex_weights"
#define VCOUNT_TAG				"vcount"
#define V_TAG					"v"

#define JOINT_SEM				"JOINT"
#define WEIGHT_SEM				"WEIGHT"
#define INV_BIND_MATRIX_SEM		"INV_BIND_MATRIX"

ControllerImporter::ControllerImporter(void)
{
}

ControllerImporter::~ControllerImporter(void)
{
	for(SKIN_CONTROLLER_MAP::iterator it = mSkinControllers.begin();
		it != mSkinControllers.end(); ++it)
	{
		DELETE_PTR(it->second->skin);
		DELETE_PTR(it->second);
	}
}

void ControllerImporter::importObject(pugi::xml_node node)
{
	if(String(node.name()) != CONTROLLER_TAG) return;

	UniqueId controllerId = node.attribute(ID_ATTR.c_str()).value();

	pugi::xml_node skinNode = node.child(SKIN_TAG);
	pugi::xml_node inputNode;
	int i = 0;

	//!!! for now support only skin controller
	//TODO: implement support of MORPH controller
	if(!skinNode) return;

	UniqueId geomIdLink = skinNode.attribute(SOURCE_ATTR.c_str()).value();
	UniqueId geomId = geomIdLink.substr(1);//skip first character as it is '#'

	//read bind shape matrix

	pugi::xml_node bind_shape_matrixNode = skinNode.child(BIND_SHAPE_MATRIX_TAG);
	ASSERT(bind_shape_matrixNode);

	FloatArray bindShapeMatrixData(16, bind_shape_matrixNode.child_value());
	Math::mat4 bindShapeMatrix(bindShapeMatrixData.getArray());

	//read data sources

	std::map<UniqueId, SourceNode *> dataArrays;

	//parse sources
	for(pugi::xml_node srcNode = skinNode.child(SOURCE_TAG.c_str()); srcNode; srcNode = srcNode.next_sibling(SOURCE_TAG.c_str()))
	{
		SourceNode * sourceNode = readSourceNode(srcNode);

		if(sourceNode)
		{
			dataArrays[sourceNode->id] = sourceNode;
		}
	}

	//read joints binding data

	pugi::xml_node jointsNode = skinNode.child(JOINTS_TAG);

	NameArray * jointNamesArr		= NULL;
	FloatArray * invBindMatricesArr	= NULL;

	for(inputNode = jointsNode.child(INPUT_TAG.c_str()); inputNode; inputNode = inputNode.next_sibling(INPUT_TAG.c_str()))
	{
		String semantic = inputNode.attribute(SEMANTIC_ATTR.c_str()).value();

		UniqueId sourceIdLink = inputNode.attribute(SOURCE_ATTR.c_str()).value();
		UniqueId sourceId = sourceIdLink.substr(1);//skip first character as it is '#'

		ASSERT(dataArrays.find(sourceId) != dataArrays.end());

		SourceNode * sourceNode = dataArrays[sourceId];

		if(semantic == JOINT_SEM) {
			jointNamesArr = sourceNode->namesArray;
			ASSERT(jointNamesArr != NULL);
		} 
		else if(semantic == INV_BIND_MATRIX_SEM) {
			invBindMatricesArr = sourceNode->floatArray;
			ASSERT(invBindMatricesArr != NULL);
		}
	}

	//read verex weights data

	pugi::xml_node vertexWeightsNode = skinNode.child(VERTEX_WEIGHTS_TAG);
	int verticesCount = vertexWeightsNode.attribute(COUNT_ATTR.c_str()).as_int();//same as vertex num in target mesh

	FloatArray * weightsArr	= NULL;

	int jointsOffset = 0;
	int weightsOffset = 1;
	int maxOffset = 0;

	for(inputNode = vertexWeightsNode.child(INPUT_TAG.c_str()); inputNode; inputNode = inputNode.next_sibling(INPUT_TAG.c_str()))
	{
		String semantic = inputNode.attribute(SEMANTIC_ATTR.c_str()).value();
		int offset = inputNode.attribute(OFFSET_ATTR.c_str()).as_int();
		if(offset > maxOffset) maxOffset = offset;

		UniqueId sourceIdLink = inputNode.attribute(SOURCE_ATTR.c_str()).value();
		UniqueId sourceId = sourceIdLink.substr(1);//skip first character as it is '#'

		ASSERT(dataArrays.find(sourceId) != dataArrays.end());

		SourceNode * sourceNode = dataArrays[sourceId];

		if(jointNamesArr == NULL && semantic == JOINT_SEM) {
			jointNamesArr = sourceNode->namesArray;
			ASSERT(jointNamesArr != NULL);
			jointsOffset = offset;
		} 
		else if(semantic == WEIGHT_SEM) {
			weightsArr = sourceNode->floatArray;
			ASSERT(weightsArr != NULL);
			weightsOffset = offset;
		}
	}

	ASSERT(maxOffset == 1);//

	pugi::xml_node vcountNode = vertexWeightsNode.child(VCOUNT_TAG);
	ASSERT(vcountNode);

	IntArray bonesPerVertexArr(verticesCount, vcountNode.child_value());
	int indicesArrCount = 0;
	for(i = 0; i < verticesCount; ++i)
	{
		indicesArrCount += bonesPerVertexArr.getElement(i);
	}
	int indicesArrStride = (maxOffset + 1);
	int indicesArrSize = indicesArrCount * indicesArrStride;

	pugi::xml_node vNode = vertexWeightsNode.child(V_TAG);
	ASSERT(vNode);

	IntArray indicesArr(indicesArrSize, vNode.child_value());
	indicesArr.setAccessorInfo(indicesArrCount, indicesArrStride);

	//all data is collected, creating skin

	Resource::Skin * skin = new Resource::Skin();

	//copy bone names to skin
	skin->boneNames.setCount(jointNamesArr->getSize());
	for(i = 0; i < jointNamesArr->getSize(); ++i)
		skin->boneNames.getData()[i] = jointNamesArr->getArray()[i];

	//create skin controller
	SkinController * skinController = new SkinController();
	skinController->id = controllerId;
	skinController->skin = skin;
	skinController->targetId = geomId;

	mSkinControllers[ controllerId ] = skinController;

	int bonesNum = jointNamesArr->getSize();

	//import bind matrices for future skeleton

	skinController->invBindMatrices.setCount( bonesNum );//tmp

	for(i = 0; i < bonesNum; ++i)
	{
		Math::mat4 inverseBindMatrix(invBindMatricesArr->getTuple(i));
		skinController->invBindMatrices.getData()[i] = inverseBindMatrix;//tmp
		skinController->invBindMatrices.getData()[i] *= bindShapeMatrix;
	}

	//import weights

	int weightsNum = weightsArr->getSize();

	skin->weights.setCount( weightsNum );

	for(i = 0; i < weightsNum; ++i)
	{
		skin->weights.getData()[i] = weightsArr->getElement(i);
	}

	//import joints

	skin->joints.setCount( verticesCount );

	int indicesArrNdx = 0;
	for(i = 0; i < verticesCount; ++i)
	{
		int bonesPerVertex = bonesPerVertexArr.getElement(i);
		Resource::Skin::Joint& joint = skin->joints.getData()[i];
		joint.setCount( bonesPerVertex );

		for(int j = 0; j < bonesPerVertex; ++j)
		{
			int * indices = indicesArr.getTuple(indicesArrNdx);

			joint.getData()[j].x = indices[weightsOffset];
			joint.getData()[j].y = indices[jointsOffset];

			//QnD fix for blender collada exporter
			if(joint.getData()[j].x < 0) 
				joint.getData()[j].x = 0;
			if(joint.getData()[j].y < 0) 
				joint.getData()[j].y = 0;

			++indicesArrNdx;
		}
	}

	//cleanup
	for(std::map<UniqueId, SourceNode *>::iterator itDA = dataArrays.begin(); itDA != dataArrays.end(); ++itDA)
	{
		DELETE_PTR(itDA->second);
	}
}

SkinController * ControllerImporter::getSkinController(const UniqueId& skinControllerId)
{
	//get skin controller
	SKIN_CONTROLLER_MAP::iterator itSkinController = mSkinControllers.find( skinControllerId );
	if(itSkinController == mSkinControllers.end()) return NULL;

	return itSkinController->second;
}

}//namespace DAEImport { 
}//namespace Squirrel {