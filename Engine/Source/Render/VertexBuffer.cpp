#include "VertexBuffer.h"

namespace Squirrel {

namespace RenderData { 

#define ADD_VC(vc)	{ if(mVertType & VCI2VT((vc))) addComponent<(vc)>(); }

//TODO: deprecate m_bAreVertsExternal as external VB realized through Model<->Mesh
VertexBuffer::VertexBuffer(int vertType, size_t vertNum, byte *pVerts):
		mVertType(vertType), mVertNum(vertNum), mVertSize(0), mVCNum(0), 
		mVerts(pVerts)
{
	memset(&mVCSizes, 0, sizeof(mVCSizes));
	memset(&mVCOffsets, 0, sizeof(mVCOffsets));

	//TODO: remove this hardcode by creating MeshVertexBufferImpl template class
	ADD_VC(vcPosition);
	ADD_VC(vcNormal);
	ADD_VC(vcTexcoord);
	ADD_VC(vcTangentBinormal);
	ADD_VC(vcInt8Normal);
	ADD_VC(vcInt8TangentBinormal);
	ADD_VC(vcTexcoord2);
	ADD_VC(vcInt8Texcoord);
	ADD_VC(vcInt8Texcoord2);
	ADD_VC(vcInt16Texcoord);
	ADD_VC(vcInt16Texcoord2);
	ADD_VC(vc4Int8BoneIndices);
	ADD_VC(vc4FloatBoneIndices);
	ADD_VC(vc4BoneWeights);
	ADD_VC(vc2BoneWeights);
	ADD_VC(vcColor);
	
	size_t szVertsSize = mVertSize*mVertNum;
	mVerts = new byte[szVertsSize];
	if(pVerts!=NULL)
	{
		memcpy(mVerts, pVerts, szVertsSize);
	}
}

VertexBuffer::~VertexBuffer(void)
{
	DELETE_ARR(mVerts);
}

//returns number of copied components
size_t VertexBuffer::Copy(VertexBuffer * src, VertexBuffer * dst)
{
	size_t vertsNum = Math::minValue( src->getVertsNum(), dst->getVertsNum() );
	size_t copiedComponents = 0;

	for(int component = 0; component < vcNum; ++component)
	{
		//if dst has no such VC then skip it
		if(!dst->hasComponent( component )) continue;

		size_t componentOffset	= src->mVCOffsets[ component ];
		size_t componentSize	= src->mVCSizes[ component ];

		for(int i = 0; i < vertsNum; ++i)
		{
			byte * srcVC = src->getVertexAddr( i ) + componentOffset;
			byte * dstVC = dst->getVertexAddr( i ) + componentOffset;
			memcpy(dstVC, srcVC, componentSize);
		}

		++copiedComponents;
	}

	return copiedComponents;
}

void VertexBuffer::MoveContent(VertexBuffer * src, VertexBuffer * dst)
{
	//move vertex data
	DELETE_ARR( dst->mVerts );
	dst->mVerts = src->mVerts;
	src->mVerts = 0;

	//move members

	dst->mVertType	= src->mVertType;
	dst->mVertNum	= src->mVertNum;
	dst->mVertSize	= src->mVertSize;
	dst->mVCNum		= src->mVCNum;
	memcpy(&dst->mVCOffsets,	&src->mVCOffsets,	sizeof(src->mVCOffsets));
	memcpy(&dst->mVCSizes,		&src->mVCSizes,		sizeof(src->mVCSizes));
	//skip mStorageType
}

void VertexBuffer::resize(size_t szNewVertNum)
{
	ASSERT(szNewVertNum > 0);
	
	size_t szOldBufferSize = mVertNum * getVertexSize();
	size_t szNewBufferSize = szNewVertNum * getVertexSize();

	byte * pNewVerts = new byte[szNewBufferSize];

	size_t szBufferSizeToCopy = Math::minValue(szOldBufferSize, szNewBufferSize);
	memcpy(pNewVerts, mVerts, szBufferSizeToCopy);

	DELETE_ARR(mVerts);

	mVerts = pNewVerts;
	mVertNum = szNewVertNum;
}

}//namespace RenderData { 

}//namespace Squirrel {

