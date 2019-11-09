#include "DocumentImporter.h"

#include <fstream>
#include <time.h>

#include <Common/Settings.h>
#include <Common/Log.h>

namespace Squirrel {

using namespace Resource;
using namespace RenderData;

namespace FBXImport { 

const int TRIANGLE_VERTEX_COUNT = 3;

// Three floats for every position.
const int VERTEX_STRIDE = 3;
// Three floats for every normal.
const int NORMAL_STRIDE = 3;
// Two floats for every UV.
const int UV_STRIDE = 2;

DocumentImporter::DocumentImporter(Render::IRender * render, Resource::TextureStorage * texStorage, Resource::MaterialLibrary * matLib):
	Resource::IModelImporter(render, texStorage, matLib), mModel(NULL)
{

}

DocumentImporter::~DocumentImporter(void)
{
}

const char_t * DocumentImporter::getImportingStageDesc() const 
{
	return mImportingStage.c_str();
}

void DocumentImporter::setImportStage(const char_t * str)
{
	mImportingStage = str;
}

bool DocumentImporter::load(const char_t * importFileName)
{	
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    FbxManager * pManager = FbxManager::Create();
    if( !pManager )
    {
		Log::Instance().error("FBXImport::DocumentImporter::load","Error: Unable to create FBX Manager!");
		return false;
    }
	//else Log::Instance().stream("FBXImport::DocumentImporter::load") << "Autodesk FBX SDK version " << pManager->GetVersion();

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

    // Set the import states. By default, the import states are always set to 
    // true. The code below shows how to change these states.
    ios->SetBoolProp(IMP_FBX_MATERIAL,        true);
    ios->SetBoolProp(IMP_FBX_TEXTURE,         true);
    ios->SetBoolProp(IMP_FBX_LINK,            true);
    ios->SetBoolProp(IMP_FBX_SHAPE,           true);
    ios->SetBoolProp(IMP_FBX_GOBO,            true);
    ios->SetBoolProp(IMP_FBX_ANIMATION,       true);
    ios->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

    // Create an importer using our sdk manager.
    FbxImporter* pImporter = FbxImporter::Create(pManager,"");
    
    // Use the first argument as the filename for the importer.
    if(!pImporter->Initialize(importFileName, -1, pImporter->GetIOSettings())) {
		Log::Instance().streamError("FBXImport::DocumentImporter::load") << "Call to FbxImporter::Initialize() failed. Error returned: " << pImporter->GetLastErrorString();
		Log::Instance().flush();
		return false;
    }
    
    // Create a new scene so it can be populated by the imported file.
    FbxScene* pScene = FbxScene::Create(pManager,"scene");

    // Import the contents of the file into the scene.
    pImporter->Import(pScene);

    // The file has been imported; we can get rid of the importer.
    pImporter->Destroy();

 	//TODO: create model outside importer and pass it through load method
	mModel = new Model();

	// convert scene into SqModel
	bool success = importScene(pScene);

   // Destroy the sdk manager and all other objects it was handling.
    pManager->Destroy();

	return success;
}

bool DocumentImporter::load(Data * data)
{	
	//TODO: !!!
	return false;
}

void convertMatrix(FbxAMatrix& srcMatrix, Math::mat4& dstMatrix)
{
	for(int i = 0; i < 4; ++i)
		for(int j = 0; j < 4; ++j)
			dstMatrix[i][j] = static_cast<float>(srcMatrix.Get(i, j));
}

Math::vec3 convertVector(FbxVector4& vec)
{
	return Math::vec3(static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2]));
}

Math::vec2 convertVector(FbxVector2& vec)
{
	return Math::vec2(static_cast<float>(vec[0]), static_cast<float>(vec[1]));
}

// Triangulate all NURBS, patch and mesh under this node recursively.
void TriangulateRecursive(FbxNode* pNode)
{
    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

    if (lNodeAttribute)
    {
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
            lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
            lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
            lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
        {
            FbxGeometryConverter lConverter(pNode->GetFbxManager());
            lConverter.TriangulateInPlace(pNode);
        }
    }

    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        TriangulateRecursive(pNode->GetChild(lChildIndex));
    }
}

bool DocumentImporter::importScene(FbxScene * scene)
{
	// Convert Axis System to what is used in this example, if needed
	FbxAxisSystem SceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if( SceneAxisSystem != OurAxisSystem )
	{
	    OurAxisSystem.ConvertScene(scene);
	}
	
	/*
	// Convert Unit System to what is used in this example, if needed
	//FbxSystemUnit SceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();
	float unitsInMeterInternal = Settings::Default()->getFloat("World", "UnitsInMeter", 10.0f);
	double centimetersInUnit = 100.0 * unitsInMeterInternal; 
	//if( SceneSystemUnit.GetScaleFactor() != centimetersInUnit )
	{
		FbxSystemUnit OurUnitSystem(centimetersInUnit);
	    //The unit in this example is centimeter.
	    OurUnitSystem.ConvertScene( scene);
	}
	*/

    // Import the nodes of the scene and their attributes recursively.
    // Note that we are not importing the root node, because it should
    // not contain any attributes.
    FbxNode* fbxRootNode = scene->GetRootNode();
    if(fbxRootNode) {

		TriangulateRecursive(fbxRootNode);

		FbxAMatrix& fbxLocalTransform = fbxRootNode->EvaluateLocalTransform();

		Math::mat4 rootTransform;

		convertMatrix(fbxLocalTransform, rootTransform);

		mModel->setTransform( rootTransform );

        for(int i = 0; i < fbxRootNode->GetChildCount(); i++)
            importNode(fbxRootNode->GetChild(i), NULL);
    }

	return true;
}

void DocumentImporter::importNode(FbxNode * fbxNode, Model::Node * parentNode)
{
	Resource::Model::Node* modelNode = mModel->addNode(parentNode, false /*nodeAnim != NULL*/);

	FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform();

	convertMatrix(fbxLocalTransform, modelNode->mTransform);

	modelNode->mName = fbxNode->GetName();

	FbxNodeAttribute* lNodeAttribute = fbxNode->GetNodeAttribute();
	if (lNodeAttribute)
	{
	    // Bake mesh as VBO(vertex buffer object) into GPU.
	    if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	    {
	        FbxMesh * lMesh = fbxNode->GetMesh();
	        if (lMesh && !lMesh->GetUserDataPtr())
	        {
				Resource::Mesh * mesh = importMesh(lMesh, modelNode);
	            if (mesh != NULL)
	            {
	                lMesh->SetUserDataPtr(mesh);
	            }
	        }
	    }
	}

	// Print the node's attributes.
    //for(int i = 0; i < node->GetNodeAttributeCount(); i++)
    //    PrintAttribute(pNode->GetNodeAttributeByIndex(i));

    // Recursively import the children nodes.
    for(int j = 0; j < fbxNode->GetChildCount(); j++)
        importNode(fbxNode->GetChild(j), modelNode);
}

Resource::Mesh * DocumentImporter::importMesh(const FbxMesh *pMesh, Resource::Model::Node * modelNode)
{
	bool mHasNormal(false), mHasUV(false), mAllByControlPoint(true);

    if (!pMesh->GetNode())
        return NULL;

    const int lPolygonCount = pMesh->GetPolygonCount();

	int i = 0;

    // Count the polygon count of each material
    FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
    FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
    if (pMesh->GetElementMaterial())
    {
        lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
        lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
            if (lMaterialIndice->GetCount() == lPolygonCount)
            {
                // Count the faces of each material
                for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                {
                    const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
					for(i = modelNode->mMatLinks.size(); i < lMaterialIndex + 1; ++i)
					{
						modelNode->mMatLinks.push_back(Model::MaterialLink());
						modelNode->mMatLinks.back().mIndexCount = 0;
					}
					modelNode->mMatLinks[lMaterialIndex].mIndexCount += TRIANGLE_VERTEX_COUNT;
                }

                // Record the offset (how many vertex)
                const int lMaterialCount = modelNode->mMatLinks.size();
                int lOffset = 0;
                for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                {
					modelNode->mMatLinks[lIndex].mIndexOffset = lOffset;
                    lOffset += modelNode->mMatLinks[lIndex].mIndexCount;
                    // This will be used as counter in the following procedures, reset to zero
                    modelNode->mMatLinks[lIndex].mIndexCount = 0;
                }
                FBX_ASSERT(lOffset == lPolygonCount * 3);
            }
        }
    }

    // All faces will use the same material.
    if (modelNode->mMatLinks.size() == 0)
    {
		modelNode->mMatLinks.push_back(Model::MaterialLink());
		modelNode->mMatLinks.back().mIndexCount = 0;
    }

	// Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    mHasNormal = pMesh->GetElementNormalCount() > 0;
    mHasUV = pMesh->GetElementUVCount() > 0;
	FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
    if (mHasNormal)
    {
        lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        if (lNormalMappingMode == FbxGeometryElement::eNone)
        {
            mHasNormal = false;
        }
        if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasUV)
    {
        lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        if (lUVMappingMode == FbxGeometryElement::eNone)
        {
            mHasUV = false;
        }
        if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }

    // Allocate the array memory, by control point or by polygon vertex.
    int lPolygonVertexCount = pMesh->GetControlPointsCount();
    if (!mAllByControlPoint)
    {
        lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
    }
	int iVertType = VCI2VT(VertexBuffer::vcPosition);
	iVertType |= VCI2VT(VertexBuffer::vcNormal);//add normals anyway as if there are no normals then need to gen them
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    const char * lUVName = NULL;
    if (mHasUV && lUVNames.GetCount())
    {
		iVertType |= VCI2VT(VertexBuffer::vcTexcoord);
        lUVName = lUVNames[0];
    }

	//create mesh
	Resource::Mesh * mesh = new Resource::Mesh;
	int vertsNum = lPolygonVertexCount;
	int indsNum = lPolygonCount * TRIANGLE_VERTEX_COUNT;
	IndexBuffer * ib = mesh->createIndexBuffer(indsNum, vertsNum < POW_2_16 ? IndexBuffer::Index16 : IndexBuffer::Index32);
	ib->setPolyType(IndexBuffer::ptTriangles);
	VertexBuffer * vb = mesh->createVertexBuffer(iVertType, vertsNum);

	//add mesh to model
	mModel->getMeshes()->push_back( mesh );

	//attach mesh to material links
	for(i = 0; i < modelNode->mMatLinks.size(); ++i)
	{
		modelNode->mMatLinks[i].mMesh = mesh;
		modelNode->mMatLinks[i].mMeshId = mModel->getMeshes()->size() - 1;
	}

    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector2 lCurrentUV;
    if (mAllByControlPoint)
    {
        const FbxGeometryElementNormal * lNormalElement = NULL;
        const FbxGeometryElementUV * lUVElement = NULL;
        if (mHasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        if (mHasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
        {
            // Save the vertex position.
            lCurrentVertex = lControlPoints[lIndex];
			vb->setComponent<VertexBuffer::vcPosition>(lIndex, convertVector(lCurrentVertex));

            // Save the normal.
            if (mHasNormal)
            {
                int lNormalIndex = lIndex;
                if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
				vb->setComponent<VertexBuffer::vcNormal>(lIndex, convertVector(lCurrentNormal));
            }

            // Save the UV.
            if (mHasUV)
            {
                int lUVIndex = lIndex;
                if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
				vb->setComponent<VertexBuffer::vcTexcoord>(lIndex, convertVector(lCurrentUV));
            }
        }
    }

    int lVertexCount = 0;
    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        // The material for current face.
        int lMaterialIndex = 0;
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
        }

        // Where should I save the vertex attribute index, according to the material
		const int lIndexOffset = modelNode->mMatLinks[lMaterialIndex].mIndexOffset +
			modelNode->mMatLinks[lMaterialIndex].mIndexCount;
        for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
        {
            const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

            if (mAllByControlPoint)
            {
				ib->setIndex(lIndexOffset + lVerticeIndex, static_cast<unsigned int>(lControlPointIndex));
            }
            // Populate the array with vertex attribute, if by polygon vertex.
            else
            {
				ib->setIndex(lIndexOffset + lVerticeIndex, static_cast<unsigned int>(lVertexCount));

                lCurrentVertex = lControlPoints[lControlPointIndex];
				vb->setComponent<VertexBuffer::vcPosition>(lVertexCount, convertVector(lCurrentVertex));

                if (mHasNormal)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
					vb->setComponent<VertexBuffer::vcNormal>(lVertexCount, convertVector(lCurrentNormal));
                }

                if (mHasUV)
                {
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV);
					vb->setComponent<VertexBuffer::vcTexcoord>(lVertexCount, convertVector(lCurrentUV));
                }
            }
            ++lVertexCount;
        }
		modelNode->mMatLinks[lMaterialIndex].mIndexCount += TRIANGLE_VERTEX_COUNT;
    }

	return mesh;
}

}//namespace FBXImport { 
}//namespace Squirrel {