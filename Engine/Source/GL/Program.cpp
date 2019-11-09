// Program.cpp: implementation of the Program class.
//
//////////////////////////////////////////////////////////////////////

#include "Program.h"
#include "Utils.h"
#include <Render/VertexBuffer.h>
#include <Common/TimeCounter.h>
#include <common/Log.h>
#include <sys/stat.h>
#include <list>
#include <set>
#include <algorithm>

namespace Squirrel {

using RenderData::VertexBuffer;

namespace GL {
	
enum GLSLQualifierKind {
	GLSLUnknownQualifierKind,
	GLSLTypeQualifierKind,
	GLSLStorageQualifierKind,
	GLSLPrecisionQualifierKind,
	GLSLInterpolationQualifierKind,
	GLSLLayoutQualifierKind,
	GLSLInvariantQualifierKind,
	GLSLPreciseQualifierKind,
	GLSLQualifierKindsNum
};
	
const char * GLSLTypeQualifiers[] = {
	"void",
	"bool",
	"int", "uint",
	"float",
	"double",
	"vec2", "vec3", "vec4",
	"mat2", "mat3", "mat4",
	"mat2x2", "mat2x3", "mat2x4",
	"mat3x2", "mat3x3", "mat3x4",
	"mat4x2", "mat4x3", "mat4x4",
	"sampler1D", "sampler2D", "sampler2D",
	"samplerCube", "sampler2DRect",
	"sampler1DShadow", "sampler2DShadow",
	"sampler2DRectShadow",
	"sampler1DArray", "sampler2DArray",
	"sampler1DArrayShadow", "sampler2DArrayShadow",
	"samplerBuffer",
	"sampler2DMS", "sampler2DMSArray",
	NULL
};
	
const char * GLSLVecMatTypePrefixes = "iubd";
const char * GLSLSamplerTypePrefixes = "iu";
	
const char * GLSLStorageQualifiers[] = {
	"const",
	"uniform",
	"varying",
	"attribute"
	"in", "out",
	"centroid in", "centroid out",
	"sample in", "sample out",
	"centroid varying",
	"patch in", "patch out",
	NULL
};

const char * GLSLLayoutQualifier	= "layout";
const char * GLSLInvariantQualifier	= "invariant";
const char * GLSLPreciseQualifier	= "precise";

const char * GLSLPrecisionQualifiers[] = {
	"lowp",
	"mediump",
	"highp",
	NULL
};
	
const char * GLSLInterpolationQualifiers[] = {
	"smooth",
	"flat",
	"noperspective",
	NULL
};
	
struct GLSLQualifier
{
	const int kind;
	const char** qualifiers;
};
	
static const struct GLSLQualifier GLSLQualifiersMap[]=
{
	{ GLSLTypeQualifierKind,			GLSLTypeQualifiers},
	{ GLSLStorageQualifierKind,			GLSLStorageQualifiers},
	{ GLSLPrecisionQualifierKind,		GLSLPrecisionQualifiers},
	{ GLSLInterpolationQualifierKind,	GLSLInterpolationQualifiers},
	{ GLSLLayoutQualifierKind,			&GLSLLayoutQualifier},
	{ GLSLInvariantQualifierKind,		&GLSLInvariantQualifier},
	{ GLSLPreciseQualifierKind,			&GLSLPreciseQualifier},
	{ static_cast<int>(0),		NULL } /* end of list indicator */
};
	
const char * GLSLSupportOfGLSL_ES =
	"#define lowp"
	"#define meduimp"
	"#define highp"
	"";
	
const char * GLSLVS_10_SupportOfGLSL_15 =
	"#define attribute in"
	"#define varying out"
	"";
	
const char * GLSLPS_10_SupportOfGLSL_15 =
	"out vec4 gl_FragColor;"
	"#define varying in"
	"";
	
void findInputs(const std::string& source, const char_t * tag, std::list<std::string>& output)
{
	const char_t * spaces = " \r\n\t";
	const char_t * nextLine = "\n";
	
	std::list<std::string> lines;
	split(source, nextLine, lines);

	for(std::list<std::string>::iterator itLine = lines.begin();
		itLine != lines.end(); ++itLine)
	{
		//remove ; from line
		std::string line = (*itLine).substr(0, (*itLine).find(';'));

		std::list<std::string> lineParts;
		split(line, spaces, lineParts);
		
		if(lineParts.size() > 2 && lineParts.front() == tag)
		{
			lineParts.pop_front();
			lineParts.pop_front();
				
			output.push_back(lineParts.front());
		}
	}
}
	
void findInputLines(const std::string& source, const char_t * tag, std::list<std::string>& output)
{
	const char_t * spaces = " \r\n\t";
	const char_t * nextLine = "\n";
	
	std::list<std::string> lines;
	split(source, nextLine, lines);
	
	for(std::list<std::string>::iterator itLine = lines.begin();
		itLine != lines.end(); ++itLine)
	{
		std::string line = (*itLine);
		
		std::list<std::string> lineParts;
		split(line, spaces, lineParts);
		
		for(std::list<std::string>::iterator itLinePart = lineParts.begin();
			itLinePart != lineParts.end(); ++itLinePart)
		{
			if((*itLinePart) == tag)
			{
				output.push_back(line);
			}
		}
	}
}

std::string cleanUpGLSLDeclarationLine(std::string inLine)
{
	//remove ; from line
	std::string line = inLine.substr(0, inLine.find(';'));
	
	//remove array size specificator if exists
	size_t pos = line.find('[');
	if(pos != std::string::npos)
	{
		line = line.substr(0, pos);
	}
	
	//remove layout qualifier and it's brackets
	size_t layoutQualifierPos = line.find(GLSLLayoutQualifier);
	if (layoutQualifierPos != std::string::npos) {
		size_t openBracketPos = line.find_first_of('(', layoutQualifierPos);
		size_t closeBracketPos = line.find_last_of(')', layoutQualifierPos);
		
		if(openBracketPos != std::string::npos && closeBracketPos != std::string::npos)
		{
			size_t length = closeBracketPos - layoutQualifierPos;
			line = line.substr(layoutQualifierPos, length);
		}
	}
	
	//remove assignment and default value
	size_t assignPos = line.find_last_of('=');
	if(assignPos != std::string::npos) {
		line = line.substr(0, assignPos);
	}
		
	return line;
}

std::list<std::string>::const_iterator findInList(const std::list<std::string>& list, const char_t * what)
{
	for(std::list<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((*it) == what)
			return it;
	}
	return list.end();
}

GLenum getShaderType(IProgram::ShaderType type)
{
	switch(type)
	{
	case IProgram::shaderVertex:		return GL_VERTEX_SHADER;
	case IProgram::shaderPixel:			return GL_FRAGMENT_SHADER;
	case IProgram::shaderGeometry:
	case IProgram::shaderTessEval:
	case IProgram::shaderTessControl:
	case IProgram::shaderUnknown:
	default:
		break;
	}
	//ASSERT(false);
	return 0;
}

void logShaderInfoLog(GLuint obj, const char_t * title, const char_t * separateLogFileName)
{
	int sourceLen = 0;
    int infologLength = 0;
    int charsWritten  = 0;
    char_t *infoLog = NULL;
    char_t *source = NULL;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength < 4)
		return;

	glGetShaderiv(obj, GL_SHADER_SOURCE_LENGTH, &sourceLen);

	source = (char_t *)malloc(sourceLen);
    glGetShaderSource(obj, sourceLen, &charsWritten, source);

    infoLog = (char_t *)malloc(infologLength);
    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);

	Log::Instance().report(title, infoLog, Log::sevImportantMessage);

	if(separateLogFileName != NULL)
	{
		std::string fname = separateLogFileName;
		size_t pos = std::string::npos;
		while((pos = fname.find_first_of("\\/")) != std::string::npos)
			fname.replace(pos, 1, "_");
		while((pos = fname.find_first_of(";")) != std::string::npos)
			fname.replace(pos, 1, " ");
		
		//if(fname.length() > 250)
		//	fname = fname.substr(0, 250);

		fname = std::string("./shaderLogs/") + fname + ".txt";

		FILE * file = fopen(fname.c_str(), "w+");
		if(file)
		{
			fprintf(file, "%s\n", source);
			fprintf(file, "\n\n%s\n", title);
			fprintf(file, ">>>>>>>>>>>>>>>>>>>>>>>>LOG:\n%s\n", infoLog);
			fclose(file);
		}
	}

	free(source);
	free(infoLog);
}

void logProgramInfoLog(GLuint obj, const char_t * title)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char_t *infoLog = NULL;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 3)
    {
        infoLog = (char_t *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
 		Log::Instance().report(title, infoLog, Log::sevImportantMessage);
		free(infoLog);
   }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Program::Program()
{
	mId = 0;
}

Program::~Program()
{
	setPool(NULL);
}

void Program::generate()
{
	//create program
	mId = glCreateProgram();
	CHECK_GL_ERROR;
}

void Program::destroy()
{
	if(mId)
	{
		glDeleteProgram(mId);
		CHECK_GL_ERROR;
		mId = 0;
	}
}

void Program::bind()
{
	if(sBoundProgram != this)
	{
		glUseProgram(mId);
		CHECK_GL_ERROR;
		sBoundProgram = this;
	}
}

void Program::unbind()
{
   Unbind();
}

void Program::Unbind()
{
	if(sBoundProgram != NULL)
	{
		glUseProgram(0);
		sBoundProgram = NULL;
	}
}

int Program::getUniformLocation(const UniformString& key)
{
	ID_DATA_MAP::iterator it = mUniformLocationsMap.find(key);
	int uniformId = -1;

	if(it != mUniformLocationsMap.end())
	{
		uniformId = it->second;
	}

	return uniformId;
}

int Program::getSamplerUnit(const UniformString& un)
{
	ID_DATA_MAP::iterator it = mUniformSamplersMap.find(un);
	if(it != mUniformSamplersMap.end())
	{
		return it->second;
	}
	return -1;
}

void Program::uniform(const UniformString& un, int u)
{
	uniformArray(un, 1, &u);
}

void Program::uniform(const UniformString& un, Math::vec3 u)
{
	uniformArray(un, 1, &u);
}

void Program::uniform(const UniformString& un, Math::vec4 u)
{
	uniformArray(un, 1, &u);
}

void Program::uniform(const UniformString& un, float u)
{
	uniformArray(un, 1, &u);
}

void Program::uniform(const UniformString& un, Math::mat3 u)
{
	uniformArray(un, 1, &u);
}

void Program::uniform(const UniformString& un, Math::mat4 u)
{
	uniformArray(un, 1, &u);
}

void Program::uniformArray(const UniformString& un, int count, const int * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		if(count == 1)
		{
			ID_DATA_MAP::iterator it = mUniformSamplersMap.find(un);
			if(it != mUniformSamplersMap.end())
			{
				mUniformSamplersMap[un] = u[0];
			}
		}

		glUniform1iv(uniformId, count, u);
		CHECK_GL_ERROR;
	}
}

void Program::uniformArray(const UniformString& un, int count, const float * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		glUniform1fv(uniformId, count, u);
		CHECK_GL_ERROR;
	}
}

void Program::uniformArray(const UniformString& un, int count, const Math::vec2 * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		glUniform2fv(uniformId, count, &u->x);
		CHECK_GL_ERROR;
	}
}
	
void Program::uniformArray(const UniformString& un, int count, const Math::vec3 * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		glUniform3fv(uniformId, count, &u->x);
		CHECK_GL_ERROR;
	}
}

void Program::uniformArray(const UniformString& un, int count, const Math::vec4 * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		glUniform4fv(uniformId, count, &u->x);
		CHECK_GL_ERROR;
	}
}

void Program::uniformArray(const UniformString& un, int count, const Math::mat4 * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		glUniformMatrix4fv(uniformId, count, GL_TRUE, &u->x.x);
		CHECK_GL_ERROR;
	}
}

void Program::uniformArray(const UniformString& un, int count, const Math::mat3 * u)
{
	int uniformId = getUniformLocation(un);
	if(uniformId >= 0)
	{
		glUniformMatrix3fv(uniformId, count, GL_TRUE, &u->x.x);
		CHECK_GL_ERROR;
	}
}

bool Program::receivesUniform(const UniformString& un) const
{
	return mUniformLocationsMap.find(un) != mUniformLocationsMap.end();
}

void Program::setSource(const std::string& source)
{
	mShaderSource = source;
}

bool Program::create(const std::string& parameters)
{
	const char_t * define = "#define ";
	const char_t paramSeparator = ';';
	const char_t * shaderTypeSufix = "_shader";
	const char_t * shaderTypePrefix = "sq_";

	mParameters = parameters;
	
	//build definesString
	std::string definesString("");
	size_t startOfParam = 0;
	size_t paramLength = 0;
	for(size_t i = 0; i < parameters.length(); ++i)
	{
		char_t ch = parameters[i];
		if(ch == paramSeparator)
		{
			definesString += define;
			definesString += parameters.substr(startOfParam, paramLength) + '\n';
			startOfParam = i + 1;
			paramLength = 0;
		}
		else 
			++paramLength;
	}

	std::string versionString = MakeString() << "#version " << Math::minValue(120, Utils::GetGLSLVersion()) << "\n";
	
	//compile shaders
	for(int shaderType = shaderUnknown; shaderType != shaderTypesNum; ++shaderType)
	{
		GLenum glType = getShaderType((ShaderType)shaderType);
		
		if(glType == 0)
			continue;
		
		std::string shaderTypeName = ShaderTypeName((ShaderType)shaderType);
		
		std::string shaderTypeMacro = std::string(shaderTypePrefix) + shaderTypeName + shaderTypeSufix;
		std::transform(shaderTypeMacro.begin(), shaderTypeMacro.end(), shaderTypeMacro.begin(), ::toupper);

		if(mShaderSource.find(shaderTypeMacro) == std::string::npos)
			continue;
		
		Log::Instance().stream("GL::Program->create", Log::sevImportantMessage) << "Compiling " << shaderTypeName + " shader...";
		
		std::string shaderTypeDefine = std::string(define) + shaderTypeMacro + '\n';
		
		std::string shaderSource = versionString + shaderTypeDefine + definesString + mShaderSource;
		
		if(!compileShader(shaderSource.c_str(), glType))
		{
			Log::Instance().error("GL::Program->create", "Failed to compile shader!");
			return false;
		}
	}

	return linkProgram();
}

bool Program::compileShader(const char *shaderData, GLenum shaderType)
{
    GLint object = glCreateShader(shaderType);
	CHECK_GL_ERROR;
    if(!object)
	{
		Log::Instance().error("GL::Program->compileShader", "Cannot create shader object!");
		return false;
	}

    GLint length = (GLint)strlen(shaderData);
    glShaderSource(object, 1, &shaderData, &length);
	CHECK_GL_ERROR;
    // compile shader object
    glCompileShader(object);
	CHECK_GL_ERROR;
    // check if shader compiled
    GLint compiled = 0;
	glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
	CHECK_GL_ERROR;

    if (!compiled)
    {
		Log::Instance().error("GL::Program->compileShader", "Cannot compile shader!");
		logShaderInfoLog(object, (std::string("Compile shader log: ") + mName).c_str(), (mName + "-" + mParameters).c_str());
    }

	if(compiled > 0)
	{
		// attach vertex shader to program object
		glAttachShader(mId, object);
		CHECK_GL_ERROR;
	}
	
	// delete object, no longer needed
	/* If a shader object is deleted while it is attached to a program object, it will be flagged for deletion, and deletion will not occur until 
	glDetachShader is called to detach it from all program objects to which it is attached.*/
	glDeleteShader(object);
	CHECK_GL_ERROR;

	return compiled != 0;
}

void Program::bindAttrib(GLuint vertexComponent, const char_t * attribName, const std::list<std::string>& attributeNames)
{
	if(findInList(attributeNames, attribName) != attributeNames.end())
	{
		mAttributes.insert(vertexComponent);
		glBindAttribLocation(mId, vertexComponent, attribName);
		CHECK_GL_ERROR;
	}
}

bool Program::hasVertexAttrib(uint vc)
{
	return mAttributes.find(vc) != mAttributes.end();
}

void Program::bindAttribs()
{
	const char_t * attrTag	= "attribute";
	const char_t * inTag	= "in";

	const std::string& source = mShaderSource;

	std::list<std::string> attributeNames;
	findInputs(source, attrTag, attributeNames);
	findInputs(source, inTag, attributeNames);

	bindAttrib(VertexBuffer::vcPosition,			"inPosition",	attributeNames);
	bindAttrib(VertexBuffer::vcNormal,				"inNormal",		attributeNames);
	bindAttrib(VertexBuffer::vcTangentBinormal,		"inTangentBinormal",	attributeNames);
//	bindAttrib(VertexBuffer::vcInt8Normal,			"inNormal",	attributeNames);
//	bindAttrib(VertexBuffer::vcInt8TangentBinormal,	"inTangentBinormal", attributeNames);
	bindAttrib(VertexBuffer::vcTexcoord,			"inTexcoord", attributeNames);
	bindAttrib(VertexBuffer::vcTexcoord2,			"inTexcoord2", attributeNames);
//	bindAttrib(VertexBuffer::vcInt8Texcoord,		"texcoord", attributeNames);
//	bindAttrib(VertexBuffer::vcInt8Texcoord2,		"texcoord2", attributeNames);
//	bindAttrib(VertexBuffer::vcInt16Texcoord,		"texcoord", attributeNames);
//	bindAttrib(VertexBuffer::vcInt16Texcoord2,		"texcoord2", attributeNames);
//	bindAttrib(VertexBuffer::vc4Int8BoneIndices,	"boneIndices", attributeNames);
	bindAttrib(VertexBuffer::vc4FloatBoneIndices,	"inBoneIndices", attributeNames);
	bindAttrib(VertexBuffer::vc4BoneWeights,		"inBoneWeights4", attributeNames);
	bindAttrib(VertexBuffer::vc2BoneWeights,		"inBoneWeights2", attributeNames);
	bindAttrib(VertexBuffer::vcColor,				"inColor", attributeNames);
}

void Program::extractUniforms()
{
	std::list<std::string> uniformLines;

	const char_t * uniformTag	= "uniform";

	findInputLines(mShaderSource, uniformTag, uniformLines);

	int uniformId = -1;
	
	int samplerCounter = 0;

	std::list<std::string>::iterator itUniformLine = uniformLines.begin();
	for(; itUniformLine != uniformLines.end(); ++itUniformLine)
	{
		std::string line = cleanUpGLSLDeclarationLine(*itUniformLine);
		
		//break down into parts
		const char_t * spaces = " \t";
		std::list<std::string> lineParts;
		split(line, spaces, lineParts);
		
		//extract name
		std::string key = lineParts.back();

		//extract type
		char ** typePtr = NULL;
		std::string type;
		std::list<std::string>::iterator itLinePart = lineParts.begin();
		for(;itLinePart != lineParts.end(); ++itLinePart)
		{
			typePtr = (char **)&GLSLTypeQualifiers[0];
			while (*typePtr != NULL) {
				if((*itLinePart) == (*typePtr))
				{
					type = (*itLinePart);
					break;
				}
				++typePtr;
			}
		}
		
		//check if type is sampler
		bool sampler = type.find("sampler") != std::string::npos;
		
		//store uniform
		uniformId = glGetUniformLocation(mId, key.c_str());
		if(uniformId >= 0)
		{
			mUniformLocationsMap[key] = uniformId;
			
			if(sampler)
			{
				mUniformSamplersMap[key] = samplerCounter;

				bind();
				
				glUniform1i(uniformId, samplerCounter);
				CHECK_GL_ERROR;

				++samplerCounter;
			}
		}
		else
		{
			//Log::Instance().streamError("GL::Program::extractUniforms")	<< "Failed to get location of uniform " << key << "!";
		}
	}
}

bool Program::linkProgram()
{
	//bind attribs to vertex shader before linking

	bindAttribs();

	//link program

    glLinkProgram(mId);
	CHECK_GL_ERROR;
    GLint linked = false;

	glGetProgramiv(mId, GL_LINK_STATUS, &linked);
	CHECK_GL_ERROR;
    if (!linked)
    {
		Log::Instance().error("GL::Program->linkProgram", "Shader failed to link!");
		//return false;
    }
	
	//extract uniforms before validation
	
	extractUniforms();

	//validate program
	
	glValidateProgram(mId);
	CHECK_GL_ERROR;
	GLint valid = false;
	glGetProgramiv(mId, GL_VALIDATE_STATUS, &valid);
	CHECK_GL_ERROR;
	if (valid == GL_FALSE)
	{
		Log::Instance().warning("GL::Program->linkProgram", "Link validation failed!");
	}

	logProgramInfoLog(mId, (std::string("Link program log: ") + mName).c_str());
	
	return linked != 0;
}

bool Program::IsSupported()
{
	return Utils::GetGLVersion().x >= 2;
}

} //namespace GL {

} //namespace Squirrel {
