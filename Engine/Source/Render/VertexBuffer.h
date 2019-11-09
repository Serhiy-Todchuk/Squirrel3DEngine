#pragma once

#include "../../../Externals/include/loki_part/Typelist.h"
#include <Common/common.h>
#include <map>
#include <vector>
#include <cstring>
#include "macros.h"
#include "IBuffer.h"

namespace Squirrel {

namespace RenderData { 

//Vertex component index to vertex type bit translation
#define VCI2VT(x)	(int(1)<<(TYPE_CAST<int>(x)))

//position only
#define VT_P		(VCI2VT(VertexBuffer::vcPosition))

//position, normal (for collisions, physic and so on)
#define VT_PN		(VCI2VT(VertexBuffer::vcPosition) | \
					VCI2VT(VertexBuffer::vcNormal))

//position, texcoord
#define VT_PT		(VT_P | VCI2VT(VertexBuffer::vcTexcoord))

//position, normal, texcoord (for simple lighting)
#define VT_PNT		(VT_PN | VCI2VT(VertexBuffer::vcTexcoord))

//position, normal, texcoord, tangent (for per-pixel lighting)
#define VT_PNTT		(VT_PNT | \
					VCI2VT(VertexBuffer::vcTangent)

//position, normal, texcoord, tangent, binormal (for per-pixel lighting)
#define VT_PNTTB	(VT_PNT | \
					VCI2VT(VertexBuffer::vcTangent) | \
					VCI2VT(VertexBuffer::vcBinormal))

//position, packedNormal...
#define VT_TERRAIN	(VCI2VT(VertexBuffer::vcPosition) | \
					VCI2VT(VertexBuffer::vcInt8Normal) | \
					VCI2VT(VertexBuffer::vcInt16Texcoord) | \
					VCI2VT(VertexBuffer::vcInt8Texcoord2))

//GUI text meshes
#define VT_TEXT		(VCI2VT(VertexBuffer::vcPosition) | \
					VCI2VT(VertexBuffer::vcTexcoord))

//GUI meshes
#define VT_GUI		(VCI2VT(VertexBuffer::vcPosition) | \
					VCI2VT(VertexBuffer::vcColor) | \
					VCI2VT(VertexBuffer::vcTexcoord))

//GUI meshes
#define VT_GUI_CLIPPED	(VT_GUI | \
						VCI2VT(VertexBuffer::vcTangentBinormal))

//software billboard
#define VT_SOFTWARE_BILLBOARDS	(VCI2VT(VertexBuffer::vcPosition) | \
								VCI2VT(VertexBuffer::vcColor) | \
								VCI2VT(VertexBuffer::vcTexcoord))

//billboard
#define VT_BILLBOARDS			(VT_SOFTWARE_BILLBOARDS | \
								VCI2VT(VertexBuffer::vcTexcoord2))


class SQRENDER_API VertexBuffer: public IBuffer
{


private:

//
// typedefs
//

	//vertex component types
	typedef	LOKI_TYPELIST_6(Math::vec3, Math::vec2, tuple2i, tuple2s, tuple4b, Math::vec4) TL_VERTEX_COMP_TYPES;

public:

	//vertex component type index
	enum VertexCompType
	{
		vctNone = -1,
		vctVec3 = 0,
		vctVec2,
		vctVec2i,
		vctVec2s,
		vctVec4b,
		vctVec4
	};

	//vertex component index
	enum VertexComponent
	{
		vcNone = -1,
		vcPosition = 0,
		vcNormal,
		vcTangentBinormal,
		vcInt8Normal,
		vcInt8TangentBinormal,
		vcTexcoord,
		vcTexcoord2,
		vcInt8Texcoord,
		vcInt8Texcoord2,
		vcInt16Texcoord,
		vcInt16Texcoord2,
		vc4Int8BoneIndices,
		vc4FloatBoneIndices,
		vc4BoneWeights,
		vc2BoneWeights,
		vcColor,
		vcNum
	};

	//helper template struct allows statically map vertex component index to vertex component type index
	//e.g. "VC2CTMapper<vcTexcoord>::type" == vctVec2
	template <int comp> struct VC2CTMapper;

	//helper template struct makes static access to real type of component by component index more easier
	//e.g. "typename VCTAccessor<vcTexcoord>::VCType" == vec2 
	template <int iComponent>
	struct VCTAccessor
	{
		typedef typename TL::TypeAt<TL_VERTEX_COMP_TYPES, VC2CTMapper<iComponent>::type >::Result VCType;
	};
	
protected:

//
// methods
//

protected:
	VertexBuffer(int vertType, size_t vertNum, byte *pVerts);
public:
	virtual ~VertexBuffer(void);

	//returns number of copied components
	static size_t Copy(VertexBuffer * src, VertexBuffer * dst);
	static void MoveContent(VertexBuffer * src, VertexBuffer * dst);
	
	inline size_t getComponentOffset(int iComponent) const
	{
		return mVCOffsets[iComponent];
	}
	
	inline byte* getVertexAddr(int index)
	{
		return (mVerts + index * mVertSize);
	}
	
	inline const byte* getVertexAddr(int index) const
	{
		return (mVerts + index * mVertSize);
	}
	
	inline byte* getComponentAddr(int index, int iComponent)
	{
		return (getVertexAddr(index) + getComponentOffset(iComponent));
	}
	
	inline const byte* getComponentAddr(int index, int iComponent) const
	{
		return (getVertexAddr(index) + getComponentOffset(iComponent));
	}
	
	template <int iComponent>
	typename VCTAccessor<iComponent>::VCType& getComponent(int index)
	{
		return *((typename VCTAccessor<iComponent>::VCType *)getComponentAddr(index, iComponent));
	}

	template <int iComponent>
	const typename VCTAccessor<iComponent>::VCType& getComponent(int index) const
	{
		return *((typename VCTAccessor<iComponent>::VCType *)getComponentAddr(index, iComponent));
	}

	template <int iComponent>
	void setComponent(int index, typename VCTAccessor<iComponent>::VCType value)
	{
		(*(typename VCTAccessor<iComponent>::VCType *)getComponentAddr(index, iComponent)) = value;
	}

	template <int iComponent>
	size_t getComponentSize() const
	{
		return sizeof(typename VCTAccessor<iComponent>::VCType);
	}

	inline bool	hasComponent(int iComp) const
	{ 
		return mVCSizes[iComp] > 0;
	}

	void resize(size_t szNewVertNum);

	inline size_t	getVertexSize()	const	{ return mVertSize; }
	inline size_t	getVertsNum()	const	{ return mVertNum; }
	inline int		getVertType()	const	{ return mVertType; }
	inline const byte*	getVerts()	const	{ return mVerts; }
	inline byte*	getVerts()	{ return mVerts; }

private:

	template <int iComponent>
	void addComponent()
	{
		++mVCNum;
		size_t szVC = getComponentSize<iComponent>();
		mVCSizes	[iComponent] = szVC;
		mVCOffsets	[iComponent] = mVertSize;
		mVertSize+=szVC;
	}

//
// members
//

protected:

	byte	*mVerts;
	int		mVertType;
	size_t	mVertNum;

private:

	size_t	mVertSize;
	int		mVCNum;

	size_t	mVCOffsets[vcNum];
	size_t	mVCSizes[vcNum];
};
	
//helper template struct allows statically map vertex component index to vertex component type index
//e.g. "VC2CTMapper<vcTexcoord>::type" == vctVec2
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcPosition	>			{ enum { type = VertexBuffer::vctVec3 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcNormal	>			{ enum { type = VertexBuffer::vctVec3 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcTexcoord	>			{ enum { type = VertexBuffer::vctVec2 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcTangentBinormal	>	{ enum { type = VertexBuffer::vctVec4 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcInt8Normal	>		{ enum { type = VertexBuffer::vctVec4b }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcInt8TangentBinormal >	{ enum { type = VertexBuffer::vctVec4b }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcTexcoord2	>			{ enum { type = VertexBuffer::vctVec2 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcInt16Texcoord	>		{ enum { type = VertexBuffer::vctVec2s }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcInt16Texcoord2	>	{ enum { type = VertexBuffer::vctVec2s }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcInt8Texcoord		>	{ enum { type = VertexBuffer::vctVec4b }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcInt8Texcoord2	>		{ enum { type = VertexBuffer::vctVec4b }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vc4Int8BoneIndices	>	{ enum { type = VertexBuffer::vctVec4b }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vc4FloatBoneIndices	>	{ enum { type = VertexBuffer::vctVec4 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vc4BoneWeights	>		{ enum { type = VertexBuffer::vctVec4 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vc2BoneWeights	>		{ enum { type = VertexBuffer::vctVec2 }; };
template <>
struct VertexBuffer::VC2CTMapper< VertexBuffer::vcColor	>				{ enum { type = VertexBuffer::vctVec4 }; };


}//namespace RenderData { 

}//namespace Squirrel {

