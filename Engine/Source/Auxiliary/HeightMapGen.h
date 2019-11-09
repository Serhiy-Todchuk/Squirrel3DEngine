// HeightMapGen.h: interface for the HeightMapGen class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <common/common.h>

namespace Squirrel {

namespace Resource {
	class Mesh;
}//namespace Resource {

namespace World {
	class HeightMap;
}//namespace Resource {

namespace Auxiliary {

template <class T>
class matrix
{
	typedef std::vector<T> VECTOR;
	typedef std::vector<VECTOR> MATRIX;

	MATRIX mMatrix;
	size_t mSize2;

public:
	matrix() {}
	matrix(size_t size_1, size_t size_2)
	{
		mMatrix.resize(size_1);
		for(size_t i = 0; i < size_1; ++i)
		{
			mMatrix[i].resize(size_2);
		}
		mSize2 = size_2;
	}

	size_t size1() { return mMatrix.size(); }
	size_t size2() { return mSize2; }

	float& operator()(size_t i1, size_t i2)
	{
		return mMatrix[i1][i2];
	}
};

class HeightMapGen  
{
	int		mResolution;// fractal resolution
	float	mRandMin;
	float	mRandMax;

	matrix<float> heightMap;

public:
	HeightMapGen(int resol);

	void gen(float randMin, float randMax);

	Resource::Mesh * buildMesh(int vertType, Math::vec3 cubeSize);
	World::HeightMap * buildTerrainHM();

	matrix<float>& getHeightMap() { return heightMap; }
	float queryHeightMap(int x, int y) { return heightMap(x, y); }

private:
	void fillFractalBase(float fracBase[5], float smoothFactor, int hmResolution);
	void fillMatrixWith(matrix<float> *mat, float value);
	void debug_printMatrix(matrix<float> *mat);

};

}//namespace Squirrel {
}//namespace Auxiliary {
