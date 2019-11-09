// Program.h: interface for the Program class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IContextObject.h"
#include <Common/Data.h>
#include <Math/mathTypes.h>
#include <string>
#include "Uniform.h"
#include "macros.h"

namespace Squirrel {

namespace Render { 

class SQRENDER_API IProgram  :
	public IContextObject,
	public UniformReceiver
{
public:
	enum ShaderType
	{
		shaderUnknown = 0,
		shaderVertex,
		shaderPixel,
		shaderGeometry,
		shaderTessEval,
		shaderTessControl,
		shaderTypesNum
	};

public:
	IProgram() {};
	virtual ~IProgram() {};

	static ShaderType ShaderTypeFromName(const std::string& name);
	static std::string ShaderTypeName(ShaderType type);

	static IProgram * GetBoundProgram() { return sBoundProgram; }

	static void Unbind() { 
		IProgram * prg = GetBoundProgram();
		if(prg != NULL)
			prg->unbind();
	}

	virtual int getSamplerUnit(const UniformString& un)		= 0;

	virtual void uniform(const UniformString& un, int u)		= 0;
	virtual void uniform(const UniformString& un, float u)		= 0;
	virtual void uniform(const UniformString& un, Math::vec3 u)	= 0;
	virtual void uniform(const UniformString& un, Math::vec4 u)	= 0;
	virtual void uniform(const UniformString& un, Math::mat3 u)	= 0;
	virtual void uniform(const UniformString& un, Math::mat4 u)	= 0;

	virtual bool create(const std::string& params)	= 0;
	virtual void bind()								= 0;
	virtual void unbind()							= 0;

	virtual void setSource(const std::string& source) = 0;

	virtual bool hasVertexAttrib(uint vc) = 0;

	//virtual void obtainUniformsFrom(UniformContainer * container) = 0;

	const std::string& getName() const		{ return mName; }
	void setName(const std::string& name)	{ mName = name; }

protected:

	std::string mName;//for debugging

	static IProgram * sBoundProgram;
};

//TODO: create IProgram.cpp and move these functions there (also create map)
inline IProgram::ShaderType IProgram::ShaderTypeFromName(const std::string& name)
{
	if(name == "vertex")		return shaderVertex;
	if(name == "fragment")		return shaderPixel;
	if(name == "geometry")		return shaderGeometry;
	if(name == "tess_eval")		return shaderTessEval;
	if(name == "tess_control")	return shaderTessControl;
	return shaderUnknown;
}

inline std::string IProgram::ShaderTypeName(IProgram::ShaderType type)
{
	switch(type)
	{
	case shaderVertex:		return "vertex";
	case shaderPixel:		return "fragment";
	case shaderGeometry:	return "geometry";
	case shaderTessEval:	return "tess_eval";
	case shaderTessControl:	return "tess_control";
	case shaderUnknown:
	default: break;
	}
	return "unknown";
}


} //namespace Render {

} //namespace Squirrel {
