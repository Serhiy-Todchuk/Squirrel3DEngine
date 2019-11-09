// Terrain.cpp: implementation of the Terrain class.
//
//////////////////////////////////////////////////////////////////////

#include "Terrain.h"
#include <Resource/TextureStorage.h>
#include <Resource/ProgramStorage.h>
#include <FileSystem/Path.h>
#include <iomanip>

#define TERRAIN_SETTINGS_SECTION "Terrain"
#define TERRAIN_AUTOGENERATION_SETTINGS_SECTION "Terrain Autogeneration"

namespace Squirrel {
namespace World {
	
HeightGenerator::HeightGenerator():
	mHMSize(4000, 400, 4000), mHMOffset(-2000, -200, -2000), mNoise(true), mNoiseScale(7, 166, 7)
{
}
	
HeightGenerator::~HeightGenerator()
{
}
	
void HeightGenerator::initSrcHM(std::string srcHMfileName, vec3 hmSize)
{
	Resource::TextureStorage * texStorage = Resource::TextureStorage::Active();
	
	std::auto_ptr<Data> imageData( texStorage->getResourceData(srcHMfileName) );
	
	if(imageData.get())
	{
		mSrcHM.reset(texStorage->getImageLoader().loadImage(imageData.get()));
	}
	
	mHMSize = hmSize;
	mHMOffset = hmSize * -0.5f;
}
	
float HeightGenerator::getHMHeight(int x, int z)
{
	x = Math::clamp<int>(x, 0, mSrcHM->getWidth() - 1);
	z = Math::clamp<int>(z, 0, mSrcHM->getHeight() - 1);
	
	byte * pixel = mSrcHM->getPixel(x, z);
	uint16 value = ((uint16 *)pixel)[0];
	
	float height = ((float)value / 65535) * mHMSize.y + mHMOffset.y;
	
	return height;
}
	
float HeightGenerator::fetchHM(float x, float z)
{
	float xScale = mHMSize.x / mSrcHM->getWidth();
	float zScale = mHMSize.z / mSrcHM->getHeight();

	x -= mHMOffset.x;
	z -= mHMOffset.z;
	
	x /= xScale;
	z /= zScale;
	
	int		xNdx	= (int)x;
	float	xFrac	= x - xNdx;
	
	int		zNdx	= (int)z;
	float	zFrac	= z - zNdx;
	
	float h1 = getHMHeight(xNdx + 0, zNdx + 0);
	float h2 = getHMHeight(xNdx + 1, zNdx + 0);
	float h3 = getHMHeight(xNdx + 0, zNdx + 1);
	float h4 = getHMHeight(xNdx + 1, zNdx + 1);
	
	float lerp1 = Math::lerp(h1, h2, xFrac);
	float lerp2 = Math::lerp(h3, h4, xFrac);
	
	return Math::lerp(lerp1, lerp2, zFrac);
}
	
void HeightGenerator::apply(HeightMap * hm, vec3 offset, vec3 scale)
{
	float noiseFactor = (float)mNoiseSeed * 0.1f;
	
	int i, j;
	for( i = 0; i < hm->getResolution().x; ++i)
	{
		for( j = 0; j < hm->getResolution().y; ++j)
		{
			float& height = hm->heightRef(i, j);
			
			float x = (i * scale.x) + offset.x;
			float z = (j * scale.z) + offset.z;
			
			if(mSrcHM.get())
			{
				float hmValue = fetchHM(x, z);
				height += hmValue;
			}
			
			if(mNoise)
			{
				float inX = Math::absValue(x / scale.x) / mNoiseScale.x;
				float inZ = Math::absValue(z / scale.z) / mNoiseScale.x;
				
				float noiseValue = mPerlinNoise.perlinNoise2D(inX, inZ, noiseFactor);
				
				noiseValue *= mNoiseScale.y;
				
				height += noiseValue;
			}
		}
	}
}
	
//////////////////////////////////////////////////////////////////////

Terrain * Terrain::sMain;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Terrain::Terrain()
{
	if(GetMain() == NULL)
	{
		setAsMain();
	}

	mLodStep = 2;
	mMaxLod = 4;

	mProgram = NULL;

	memset(&mTextures, 0, sizeof(mTextures));
	memset(&mBumps, 0, sizeof(mBumps));
}

Terrain::~Terrain()
{
	for(int i = 0; i <  MAX_TEXTURES_PER_NODE; ++i)
	{
		if(mTextures[i] != NULL)
			Resource::TextureStorage::Active()->release(mTextures[i]->getID());
		if(mBumps[i] != NULL)
			Resource::TextureStorage::Active()->release(mBumps[i]->getID());
	}

	if(mProgram)
		Resource::ProgramStorage::Active()->release(mProgram->getID());
}

void Terrain::setAsMain()
{
	sMain = this;
}

Terrain * Terrain::GetMain()
{
	return sMain;
}

void Terrain::initTextures(const char_t * texture1Name, const char_t * texture2Name, const char_t * texture3Name, const char_t * texture4Name)
{
	const char_t * textureNames[MAX_TEXTURES_PER_NODE] = { texture1Name, texture2Name, texture3Name, texture4Name };

	for(int i = 0; i < MAX_TEXTURES_PER_NODE; ++i)
	{
		if(textureNames[i] == NULL)
			continue;

		mTextures[i] = Resource::TextureStorage::Active()->loadTexture( textureNames[i] );

		if(mTextures[i] != NULL)
		{
			//create bump map
			mBumps[i] = Resource::TextureStorage::Active()->loadOrGenerateBumpHeightMap( textureNames[i] );
		}
	}
}

void Terrain::init(Settings * settings, bool autoGenerate)
{
	mProgram = Resource::ProgramStorage::Active()->add("forward/terrain.glsl");

	std::string terraPath = settings->getString(TERRAIN_SETTINGS_SECTION, "Storage", "Terrain");
	terraPath = FileSystem::Path::GetAbsPath(terraPath);

	FileSystem::FileStorage * contentSource = FileSystem::FileStorageFactory::GetFileStorageForPath( terraPath );
	if(contentSource != NULL)
	{
		mContentSource.reset( contentSource );
	}
	
	mNodesNum		= settings->getInt(TERRAIN_SETTINGS_SECTION, "Nodes Num", 3);
	mNodeSize		= settings->getFloat(TERRAIN_SETTINGS_SECTION, "Node Size", 100.0f);
	mCellsPerNode	= settings->getInt(TERRAIN_SETTINGS_SECTION, "Cells Per Node", 128);
	mLodStep		= settings->getInt(TERRAIN_SETTINGS_SECTION, "LOD Step", 1);
	mFirstLodStep	= settings->getInt(TERRAIN_SETTINGS_SECTION, "First LOD Step", 2);
	mMaxLod			= settings->getInt(TERRAIN_SETTINGS_SECTION, "Max LOD", 4);
	mStartLod		= settings->getInt(TERRAIN_SETTINGS_SECTION, "Start LOD", 0);

	if((mGenerateMissingNodes = autoGenerate))
	{
		std::string sourceHMFileName = settings->getString(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "Source Height Map", "");
		float srcHMsizeX = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "SourceHM sizeX", 1000.0f);
		float srcHMsizeY = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "SourceHM sizeY", 100.0f);
		float srcHMsizeZ = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "SourceHM sizeZ", 1000.0f);
		float srcHMoffseX = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "SourceHM offsetX", -500.0f);
		float srcHMoffseY = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "SourceHM offsetY", 50.0f);
		float srcHMoffseZ = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "SourceHM offsetZ", -500.0f);

		mHeightGen.initSrcHM(sourceHMFileName, vec3(srcHMsizeX, srcHMsizeY, srcHMsizeZ));
		mHeightGen.mHMOffset = vec3(srcHMoffseX, srcHMoffseY, srcHMoffseZ);

		float noiseScaleX = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "PerliNoise scaleX", 1.0f);
		float noiseScaleY = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "PerliNoise scaleY", 1.0f);
		float noiseScaleZ = settings->getFloat(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "PerliNoise scaleZ", 1.0f);

		mHeightGen.mNoiseSeed = settings->getInt(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "Noise Seed", rand() % POW_2_24);
		mHeightGen.mNoise = settings->getInt(TERRAIN_AUTOGENERATION_SETTINGS_SECTION, "Perlin Noise", 1) != 0;
		mHeightGen.mNoiseScale = vec3(noiseScaleX, noiseScaleY, noiseScaleZ);
	}
	
	setCenter(tuple2i(0, 0));
}

void Terrain::render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	bool mOneTexture = mTextures[1] == NULL;

	float lod = Math::maxValue( 0.0f, info.minLOD );

	Render::MaterialGroup * matGroup = renderQueue->beginMaterialGroup();

	if(info.level >= rilMaterials)
	{
		std::stringstream programParams;

		if(!mOneTexture)
			programParams << "FOUR_TEXTURES;";

		matGroup->mTextures["decalMap"]		= mTextures[0]->getRenderTexture();
		if(lod < sLODGoodForReflections)
		{
			matGroup->mTextures["normalHeightMap"]	= mBumps[0]->getRenderTexture();
			programParams << "BUMP;";
		}

		if(!mOneTexture)
		{
			if(mTextures[1] != NULL)
				matGroup->mTextures["decalMap2"] = mTextures[1]->getRenderTexture();
			if(mTextures[2] != NULL)
				matGroup->mTextures["decalMap3"] = mTextures[2]->getRenderTexture();
			if(mTextures[3] != NULL)
				matGroup->mTextures["decalMap4"] = mTextures[3]->getRenderTexture();
			
			if(lod < sLODGoodForReflections)
			{
				if(mBumps[1] != NULL)
					matGroup->mTextures["normalHeightMap2"] = mBumps[1]->getRenderTexture();
				if(mBumps[2] != NULL)
					matGroup->mTextures["normalHeightMap3"] = mBumps[2]->getRenderTexture();
				if(mBumps[3] != NULL)
					matGroup->mTextures["normalHeightMap4"] = mBumps[3]->getRenderTexture();
			}
			
			vec4 specularValues(1,1,1,1);
			matGroup->mUniformsPool.uniformArray("specularValues", 1, &specularValues);
		}

		float texCoordMult = 1.0f/16;
		matGroup->mUniformsPool.uniformArray("texCoordMult", 1, &texCoordMult);

		matGroup->mProgramName		= "forward/terrain.glsl";

		matGroup->mProgramParams	= programParams.str();
		
		//matGroup->mReliefScale	= 0.003f;

		/*
		 if(matLink.idMaterial >= 0)
		 {
		 Material * mat = NULL;

		 if(MaterialLibrary::Active() != NULL)
		 {
		 mat = MaterialLibrary::Active()->get(matLink.idMaterial);
		 }

		 if(mat == NULL && mModel != NULL)
		 {
		 mat = mModel->getLocalMaterials()->get(matLink.idMaterial);
		 }
		 
		 batch.mMaterial = mat;
		 }
		 */
	}

	matGroup = renderQueue->endMaterialGroup();

	int centerIndex = (mNodesNum - 1) / 2;

	if(mNodes[centerIndex][centerIndex].get() != NULL)
		mNodes[centerIndex][centerIndex]->render(camera, matGroup);

	for(int i = 0; i <= centerIndex; ++i)
	{
		for(int j = 0; j <= centerIndex; ++j)
		{
			if(i == 0 && j == 0)
				continue;

			if(mNodes[centerIndex + i][centerIndex + j].get() != NULL && i != 0 && j != 0)
				mNodes[centerIndex + i][centerIndex + j]->render(camera, matGroup);
			if(mNodes[centerIndex - i][centerIndex + j].get() != NULL)
				mNodes[centerIndex - i][centerIndex + j]->render(camera, matGroup);
			if(mNodes[centerIndex + i][centerIndex - j].get() != NULL)
				mNodes[centerIndex + i][centerIndex - j]->render(camera, matGroup);
			if(mNodes[centerIndex - i][centerIndex - j].get() != NULL && i != 0 && j != 0)
				mNodes[centerIndex - i][centerIndex - j]->render(camera, matGroup);
		}
	}
}
	
std::string Terrain::createHMFileName(tuple2i nodePos)
{
	std::stringstream fileNameStr;
	
	const char_t * negativeTag	= "n";
	const char_t * positiveTag	= "p";
	const char_t * extension	= ".sqhm";
	
	const ptrdiff_t width = 4;

	fileNameStr.fill('0');
	
	fileNameStr << std::setw(0) << ((nodePos.x >= 0) ? positiveTag : negativeTag);
	fileNameStr << std::setw(width) << std::right << Math::absValue(nodePos.x);
	fileNameStr << std::setw(0) << ((nodePos.y >= 0) ? positiveTag : negativeTag);
	fileNameStr << std::setw(width) << std::right << Math::absValue(nodePos.y);
	
	fileNameStr << std::setw(0) << extension;
	
	return fileNameStr.str();
}	

HeightMap * Terrain::loadHM(tuple2i gridPos)
{
	std::string fileName = createHMFileName(gridPos);
	
	vec3 scale = vec3(getCellSize(), 1.0f, getCellSize());
	
	Data * data = mContentSource->getMappedFile(fileName);
	
	HeightMap * hm = NULL;
	
	if(data == NULL)
	{
		if(mGenerateMissingNodes)
		{
			hm = new HeightMap(mCellsPerNode + 1, mCellsPerNode + 1);
			
			hm->clear();
			
			vec3 offset = getGlobalOffsetForNodePos(gridPos.x, gridPos.y);
			
			mHeightGen.apply(hm, offset, scale);
			
			hm->updateNormals();
		}
	}
	else
	{
		bool mapped = false;
		if(mapped)
		{
			hm = HeightMap::LoadMapped(data, true);
		}
		else
		{
			hm = HeightMap::Load(data);
			delete data;
		}
	}
	
	return hm;
}

RenderData::IndexBuffer * Terrain::getIndexBuffer(int size, tuple2i glue)
{
	RenderData::IndexBuffer * ib = NULL;

	int key = log((double)size - 1) / log( 2.0 );

	int xGlueIndex = glue.x + 1;
	int yGlueIndex = glue.y + 1;

	if(mIndexBuffers[key][xGlueIndex][yGlueIndex].get())
	{
		ib = mIndexBuffers[key][xGlueIndex][yGlueIndex].get();
	}
	else
	{
		ib = TerrainNode::GenIndexBuffer(tuple2i(size, size), glue);
		mIndexBuffers[key][xGlueIndex][yGlueIndex].reset(ib);
	}

	return ib;
}

TerrainNode * Terrain::makeNode(HeightMap * hm, tuple2i gridPos, int lod)
{
	TerrainNode * node = NULL;

	vec3 scale = vec3(getCellSize(), 1.0f, getCellSize());
	
	if(hm != NULL)
	{
		node = new TerrainNode();
		node->setScale(scale);
		node->init(NULL, hm, lod);
		node->setGridPos(gridPos);
	}
	
	return node;
}
	
void Terrain::saveUnsavedNodes()
{
	int i, j;//indices
	
	for(i = 0; i < mNodesNum; ++i)
	{
		for(j = 0; j < mNodesNum; ++j)
		{
			TerrainNode * node = mNodes[i][j].get();
			if(node != NULL)
			{
				std::string fileName = createHMFileName(node->getGridPos());
				
				if(!mContentSource->hasFile(fileName))
				{
					Data * data = node->getHeightMap()->save();
					mContentSource->putFile(data, fileName);
				}
			}
		}
	}
}
	
vec3 Terrain::getOffsetForNodeIndex(int x, int z)
{
	int centerIndex = (mNodesNum - 1) / 2;
	vec3 offset(x - centerIndex, 0, z - centerIndex);
	offset -= vec3(0.5f, 0, 0.5f);
	offset *= mNodeSize;
	return offset;
}

vec3 Terrain::getGlobalOffsetForNodePos(int x, int z)
{
	vec3 offset(x, 0, z);
	offset -= vec3(0.5f, 0, 0.5f);
	offset *= mNodeSize;
	return offset;
}
	
int Terrain::lodForIndex(int i, int j)
{
	i = Math::clamp(i, 0, mNodesNum - 1);
	j = Math::clamp(j, 0, mNodesNum - 1);

	int centerIndex = (mNodesNum - 1) / 2;
	int distanceFromCenter = Math::maxValue( Math::absValue(centerIndex - i), Math::absValue(centerIndex - j) );
	int lod = Math::maxValue( Math::minValue( (distanceFromCenter - mFirstLodStep) / mLodStep + 1, mMaxLod ), 0 ) + mStartLod;
	return lod;
}

void Terrain::setCenter(Squirrel::tuple2i newCenterNodePos)
{
	mCenterNodePos = newCenterNodePos;
	
	mBoundVolume.reset();
	
	saveUnsavedNodes();
	
	//store existing nodes
	
	typedef std::map<tuple3i, TerrainNode *> NODES_MAP;
	typedef std::map<tuple2i, HeightMap *> HMS_MAP;
	NODES_MAP loadedNodes;
	HMS_MAP loadedHMs;
	
	int x, z;//gridPos
	int i, j;//indices
	
	for(i = 0; i < mNodesNum; ++i)
	{
		for(j = 0; j < mNodesNum; ++j)
		{
			TerrainNode * node = mNodes[i][j].release();
			if(node != NULL)
			{
				loadedNodes[tuple3i(node->getGridPos(), node->getLOD())] = node;

				HeightMap * hm = mHMs[i][j].release();
				if(hm != NULL)
				{
					loadedHMs[node->getGridPos()] = hm;
				}
			}
		}
	}
	
	//load new nodes or use existing ones
	
	int centerIndex = (mNodesNum - 1) / 2;
	
	tuple2i startNodePos = tuple2i(newCenterNodePos.x - centerIndex, newCenterNodePos.y - centerIndex);
	
	for(x = startNodePos.x, i = 0; i < mNodesNum; ++x, ++i)
	{
		for(z = startNodePos.y, j = 0; j < mNodesNum; ++z, ++j)
		{
			int lod = lodForIndex(i, j);

			tuple2i glue(0, 0);

			int lod_ip1j0 = lodForIndex(i + 1, j);
			int lod_im1j0 = lodForIndex(i - 1, j);
			int lod_i0jp1 = lodForIndex(i, j + 1);
			int lod_i0jm1 = lodForIndex(i, j - 1);
			
			if(lod_ip1j0 > lod)
				glue.x = 1;
			if(lod_im1j0 > lod)
				glue.x = -1;
			if(lod_i0jp1 > lod)
				glue.y = 1;
			if(lod_i0jm1 > lod)
				glue.y = -1;
			
			tuple2i hmPos(x, z);
			tuple3i nodePos(hmPos, lod);
			
			TerrainNode * node = NULL;
			HeightMap * hm = NULL;

			HMS_MAP::iterator itHM = loadedHMs.find(hmPos);
			if(itHM != loadedHMs.end())
			{
				hm = itHM->second;
				loadedHMs.erase(itHM);
			}
			
			if(hm == NULL)
			{
				hm = loadHM(hmPos);
			}

			mHMs[i][j].reset(hm);

			NODES_MAP::iterator itLoaded = loadedNodes.find(nodePos);
			if(itLoaded != loadedNodes.end())
			{
				node = itLoaded->second;
				loadedNodes.erase(itLoaded);
			}
			
			if(node == NULL && hm != NULL)
			{
				node = makeNode(hm, hmPos, lod);
			}
			
			if(node != NULL)
			{
				int lodPow2 = 1 << lod;
				int ibSize = mCellsPerNode / lodPow2 + 1;
				RenderData::IndexBuffer * ib = getIndexBuffer(ibSize, glue);
				node->setIndexBuffer(ib);

				vec3 offset = getGlobalOffsetForNodePos(x, z);
				node->setOffset( offset );
				
				mBoundVolume.merge(node->getTransformedAABB());
			}
			
			mNodes[i][j].reset(node);
		}
	}
	
	//remove unused nodes and hms
	
	FOREACH(NODES_MAP::iterator, itNode, loadedNodes)
	{
		DELETE_PTR(itNode->second);
	}
	FOREACH(HMS_MAP::iterator, itHM, loadedHMs)
	{
		DELETE_PTR(itHM->second);
	}
}
	
tuple2i Terrain::getNextNodePos(vec3 beholderPos)
{
	tuple2i nextNodePos = getCenterNodePos();
	
	TerrainNode * currNode = getCenterNode();
	AABB aabb = currNode->getTransformedAABB();
	
	if(beholderPos.x > aabb.max.x)
		++nextNodePos.x;
	else if(beholderPos.x < aabb.min.x)
		--nextNodePos.x;

	if(beholderPos.z > aabb.max.z)
		++nextNodePos.y;
	else if(beholderPos.z < aabb.min.z)
		--nextNodePos.y;
	
	return nextNodePos;
}

float Terrain::height(float x, float z)
{
	const float correctionOffset = 0.1f;
	
	int centerIndex = (mNodesNum - 1) / 2;

	vec3 startPos = getGlobalOffsetForNodePos(mCenterNodePos.x, mCenterNodePos.y);
	
	float relX = x - startPos.x;
	float relZ = z - startPos.z;
	
	int i = centerIndex + (int)(relX / mNodeSize);
	int j = centerIndex + (int)(relZ / mNodeSize);
	
	i = Math::clamp(i, 0, mNodesNum);
	j = Math::clamp(j, 0, mNodesNum);
	
	TerrainNode * node = mNodes[i][j].get();
	
	if(node == NULL)
		return 0.0f;
	
	AABB aabb = node->getTransformedAABB();
	
	x = Math::clamp(x, aabb.min.x + correctionOffset, aabb.max.x - correctionOffset);
	z = Math::clamp(z, aabb.min.z + correctionOffset, aabb.max.z - correctionOffset);	
		
	return node->height(x, z);
}

}//namespace World {
}//namespace Squirrel {
