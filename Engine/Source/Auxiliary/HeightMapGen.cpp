// HeightMapGen.cpp: implementation of the HeightMapGen class.
//
//////////////////////////////////////////////////////////////////////

#include "HeightMapGen.h"
#include <Resource/Mesh.h>
#include <World/Terrain.h>
#include <Common/Data.h>
#include <sstream>

#ifdef	_WIN32
	#pragma	warning (disable:4996)
#endif

namespace Squirrel {
namespace Auxiliary {

using RenderData::IndexBuffer;
using RenderData::VertexBuffer;

HeightMapGen::HeightMapGen(int resol):
	mResolution(Math::getClosestPowerOfTwo(resol)), heightMap(mResolution + 1, mResolution + 1)
{
	
}

void HeightMapGen::fillFractalBase(float fracBase[5], float smoothFactor, int hmResolution)
{
	fracBase[0] = Math::getRandomMinMax(mRandMin,mRandMax) * (mResolution / (smoothFactor * hmResolution));
	fracBase[1] = Math::getRandomMinMax(mRandMin,mRandMax) * (mResolution / (smoothFactor * hmResolution));
	fracBase[2] = Math::getRandomMinMax(mRandMin,mRandMax) * (mResolution / (smoothFactor * hmResolution));
	fracBase[3] = Math::getRandomMinMax(mRandMin,mRandMax) * (mResolution / (smoothFactor * hmResolution));
	fracBase[4] = Math::getRandomMinMax(mRandMin,mRandMax) * (mResolution / (smoothFactor * hmResolution));
}

void HeightMapGen::debug_printMatrix(matrix<float> *mat)
{
	const int clampSize = 100;

	char tempStr[128]; 
	std::stringstream str;

	str << "Matrix size = " << mat->size1() << std::endl;

	for(uint x = 0; x < mat->size1(); ++x)
	{
		if(x >= (clampSize-1))
		{
			str << "..."; break;
		}

		for(uint y = 0; y < mat->size2(); ++y)
		{
			if(y >= clampSize)
			{
				str << "..."; break;
			}

			sprintf(tempStr, "%1.2f", mat->operator()(x, y));
			str << tempStr << "\t";
		}
		str << std::endl;
	}

//#ifdef _WIN32
	//OutputDebugString( str.str().c_str() );
//#endif

	sprintf(tempStr, "hm%d.txt", (int)mat->size1());

	Data file(NULL, str.str().length() );
	file.putString( str.str() );
	file.writeToFile(tempStr);
}

void HeightMapGen::fillMatrixWith(matrix<float> *mat, float value)
{
	for(uint x = 0; x < mat->size1(); ++x)
		for(uint y = 0; y < mat->size2(); ++y)
			(*mat)(x, y) = value;
}

void HeightMapGen::gen(float randMin, float randMax)
{
	const float smoothFactor		= 1.5f;
	const float baseSmoothFactor	= 1.0f;

	mRandMin = randMin;
	mRandMax = randMax;

	matrix<float> *currHM = NULL;
	matrix<float> *prevHM = NULL;

	int x, y;
	int currHMResol, prevHMResol;

	/*	determ fractal base	*/
	float fracBase[5];
	fillFractalBase(fracBase, baseSmoothFactor, 2);

	/*	determ 2x2 (*currHM) (3x3 grid)	*/
	currHM = new matrix<float>(3, 3);
	fillMatrixWith(currHM, 0.0f);

	/*	init 2x2 (*currHM) (3x3 grid)	*/
	(*currHM)(2, 1) = fracBase[0];
	(*currHM)(1, 2) = fracBase[1];
	(*currHM)(0, 1) = fracBase[2];
	(*currHM)(1, 0) = fracBase[3];
	(*currHM)(1, 1) = fracBase[4];

	/*	
	Gen Fractal Height Map	
	*/
	for(currHMResol = 4, prevHMResol = 2; currHMResol <= mResolution; currHMResol <<= 1, prevHMResol <<= 1)
	{
		/*	deletePHM	*/
		DELETE_PTR( prevHM );

		/*	swapCHM2PHM	*/
		prevHM = currHM;

		/*	determCHM	*/
		currHM = new matrix<float>(currHMResol + 1, currHMResol + 1);
		fillMatrixWith(currHM, 0.0f);

		/*	copyPHM2<<AHM	*/
		for(x = 0; x <= currHMResol; x += 2)
		{
			for(y = 0; y<=currHMResol; y += 2)
			{
				int x1 = x >> 1;
				int y1 = y >> 1;
				float prevHMValue = (*prevHM)(x1, y1);
				(*currHM)(x, y) = prevHMValue;
			}
		}

		/*	calc AHM heights	*/
		for(x = 1; x < currHMResol; x += 2)
		for(y = 1; y < currHMResol; y += 2)
		{
			fillFractalBase(fracBase, smoothFactor, currHMResol);

			(*currHM)(x + 1, y + 0) = (((*currHM)(x + 1, y - 1) + (*currHM)(x + 1, y + 1)) * 0.5f) + fracBase[0];
			(*currHM)(x + 0, y + 1) = (((*currHM)(x - 1, y + 1) + (*currHM)(x + 1, y + 1)) * 0.5f) + fracBase[1];
			(*currHM)(x - 1, y + 0) = (((*currHM)(x - 1, y + 1) + (*currHM)(x - 1, y - 1)) * 0.5f) + fracBase[2];
			(*currHM)(x + 0, y - 1) = (((*currHM)(x - 1, y - 1) + (*currHM)(x + 1, y - 1)) * 0.5f) + fracBase[3];

			(*currHM)(x + 0, y + 0) = (((*currHM)(x - 1, y - 1) + (*currHM)(x + 1, y - 1) + (*currHM)(x + 1, y + 1) + (*currHM)(x - 1, y + 1)) * 0.25f) + fracBase[4];

			(*currHM)(x + 1, y + 0) = (*currHM)(x - 1, y + 0);
			(*currHM)(x + 0, y + 1) = (*currHM)(x + 0, y - 1);
		}
	}

	heightMap = (*currHM);

	//debug_printMatrix( currHM );

	DELETE_PTR( prevHM );
	DELETE_PTR( currHM );
}

Resource::Mesh * HeightMapGen::buildMesh(int vertType, Math::vec3 cubeSize)
{
	Math::vec2 texCoordMapping(16,16);

	int i, j;

	Resource::Mesh * mesh = new Resource::Mesh();

	//gen vertex buffer

	size_t vertsNum = heightMap.size1() * heightMap.size2();
	VertexBuffer * vb = mesh->createVertexBuffer(vertType, (uint)vertsNum);
	ASSERT(vb->hasComponent(VertexBuffer::vcPosition));//must have positions
	
	size_t hmGridSize = heightMap.size1();//mResolution + 1
	int vertexIndex = 0;
	for(i = 0; i < hmGridSize; ++i)
	{
		for(j = 0; j < hmGridSize; ++j)
		{
			float h = heightMap(i, j);
			Math::vec3 vertPos(cubeSize.x * i, cubeSize.y * h, cubeSize.z * j);
			vb->setComponent<VertexBuffer::vcPosition>(vertexIndex, vertPos);

			if(vb->hasComponent(VertexBuffer::vcTexcoord))
			{
				Math::vec2 texCoord(texCoordMapping.x * i / hmGridSize, texCoordMapping.y * j / hmGridSize);
				vb->setComponent<VertexBuffer::vcTexcoord>(vertexIndex, texCoord);
			}

			++vertexIndex;
		}
	}

	//gen index buffer

	uint indsNum		= mResolution * mResolution * 6;
	IndexBuffer * ib = mesh->createIndexBuffer(indsNum);
	ib->setPolyType(IndexBuffer::ptTriangles);
	ib->setPolyOri(IndexBuffer::poCounterClockWise);

	int indexIndex = 0;
	for(i = 0; i < mResolution; ++i)
	{
		for(j = 0; j < mResolution; ++j)
		{
			int index1 = ((i + 0) * (int)hmGridSize) + (j + 0);
			int index2 = ((i + 0) * (int)hmGridSize) + (j + 1);
			int index3 = ((i + 1) * (int)hmGridSize) + (j + 1);
			int index4 = ((i + 1) * (int)hmGridSize) + (j + 0);

			ib->setIndex(indexIndex++, index2);
			ib->setIndex(indexIndex++, index3);
			ib->setIndex(indexIndex++, index1);
			ib->setIndex(indexIndex++, index1);
			ib->setIndex(indexIndex++, index3);
			ib->setIndex(indexIndex++, index4);
		}
	}

	//build tangent basis

	if(vb->hasComponent(VertexBuffer::vcNormal))
	{
		mesh->calcNormals(ib, vb);
	}
	if(vb->hasComponent(VertexBuffer::vcTangentBinormal))
	{
		mesh->calcTangentBasis(ib, vb);
	}

	return mesh;
}

World::HeightMap * HeightMapGen::buildTerrainHM()
{
	uint i, j;

	World::HeightMap * newHM = new World::HeightMap((int)heightMap.size1(), (int)heightMap.size2());

	for(i = 0; i < heightMap.size1(); ++i)
	{
		for(j = 0; j < heightMap.size2(); ++j)
		{
			newHM->heightRef(i,j) = heightMap(i, j);
		}
	}

	for(i = 0; i < heightMap.size1(); ++i)
	{
		for(j = 0; j < heightMap.size2(); ++j)
		{
			newHM->updateNormal(i, j);
		}
	}

	return newHM;
}

}//namespace Squirrel {
}//namespace Auxiliary {
