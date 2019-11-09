// Program.h: interface for the Program class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <map>

#include <Math/mathTypes.h>
#include <Render/IProgram.h>
#include <Common/DataMap.h>

#include "Utils.h"
#include <set>
#include <list>

namespace Squirrel {

namespace GL { 

using namespace Render;

class SQOPENGL_API Program:
	public IProgram
{
public://ctor/dtor
	Program();
	virtual ~Program();

public://methods

	//IProgram implementation

	void setSource(const std::string& source);

	bool create(const std::string& params);
	void bind();
	void unbind();	

	virtual bool hasVertexAttrib(uint vc);

	virtual int getSamplerUnit(const UniformString& un);

	void uniform(const UniformString& un, float u);
	void uniform(const UniformString& un, Math::vec3 u);
	void uniform(const UniformString& un, Math::vec4 u);
	void uniform(const UniformString& un, Math::mat3 u);
	void uniform(const UniformString& un, Math::mat4 u);
	void uniform(const UniformString& un, int u);

	//implement UniformReceiver

	void uniformArray(const UniformString& un, int count, const int * u);
	void uniformArray(const UniformString& un, int count, const float * u);
	void uniformArray(const UniformString& un, int count, const Math::vec2 * u);
	void uniformArray(const UniformString& un, int count, const Math::vec3 * u);
	void uniformArray(const UniformString& un, int count, const Math::vec4 * u);
	void uniformArray(const UniformString& un, int count, const Math::mat3 * u);
	void uniformArray(const UniformString& un, int count, const Math::mat4 * u);

	bool receivesUniform(const UniformString& un) const;

	//implement IContextObject

	virtual void generate();
	virtual void destroy();

	//static methods

	static bool IsSupported();
	static void Unbind();

private:

	bool compileShader(const char *shaderData, GLenum shaderType);
	bool linkProgram();

	void bindAttribs();
	void extractUniforms();

	int getUniformLocation(const UniformString& key);

	void bindAttrib(GLuint vertexComponent, const char_t * attribName, const std::list<std::string>& attributeNames);

	typedef UniformMap<UniformString, int> ID_DATA_MAP;
	
	std::string	mShaderSource;
	
	std::string mParameters;
	
	ID_DATA_MAP mUniformLocationsMap;
	ID_DATA_MAP mUniformSamplersMap;

	std::set<uint> mAttributes;

	GLuint mId;
};


} //namespace GL {

} //namespace Squirrel {
