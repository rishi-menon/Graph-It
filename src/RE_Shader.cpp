#include "RE_Shader.h"
#include <string>
#include <fstream>
#include <Maths.h>

#include "RE_Buffers.h"

static bool ParseShader(const char* const filePath, std::string outShaders[]);
static unsigned int CompileShader(unsigned int nShaderType, const char* strShaderCode);

Shader::~Shader() {
    glDeleteProgram(m_id);
}
Shader::Shader(const char* path) {
    bool b = Load(path);
    Assert(b);
}
bool Shader::Load(const char* path) {
    //Shader stuff here
    m_id = glCreateProgram();

    std::string strShaders[3];
    ParseShader(path, strShaders);
    unsigned int nVertex = CompileShader(GL_VERTEX_SHADER, strShaders[0].c_str());
    unsigned int nFragment = CompileShader(GL_FRAGMENT_SHADER, strShaders[1].c_str());
    bool bHasGeo = strShaders[2].size() != 0;
    unsigned int nGeo;
    if (bHasGeo) {
        nGeo = CompileShader(GL_GEOMETRY_SHADER, strShaders[2].c_str());
    }

    glAttachShader (m_id, nVertex);
    glAttachShader (m_id, nFragment);
    if (bHasGeo)
        glAttachShader (m_id, nGeo);
    
    glLinkProgram(m_id);
    glCheckError();

    int success = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if(!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(m_id, 512, NULL, infoLog);
        LogWarn("Shader failed linking: %s> %s", path, infoLog);
        glCheckError();
        return false;
    }

    glDeleteShader(nVertex);
    glDeleteShader(nFragment);
    if (bHasGeo)
        glDeleteShader(nGeo);
    
    glCheckError();

    return true;
}

void Shader::Bind() const {
    glUseProgram(m_id);
}
void Shader::Unbind() const {
    glUseProgram(0);
}

int Shader::GetUniformLocation(const char* name) const {
    int loc;
    std::unordered_map<std::string, int>::const_iterator it = mapUniforms.find(name);
    if(it == mapUniforms.end()) {
        loc = glGetUniformLocation(m_id, name); 
        if (loc == -1) {
            LogWarn("Uniform %s is not being used", name);
            return -1;
        }
        mapUniforms[name] = loc;
    }
    else {
        loc = it->second;
    }
    return loc;
    // glGetUniformLocation(nShader, "u_brick")

}
void Shader::SetInt(const char* name, int val) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform1i(loc, val);
    glCheckError();
}

void Shader::SetInts(const char* name, int* data, int count) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform1iv(loc, count, data);
    glCheckError();
}



void Shader::SetFloat(const char* name, float val) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform1f(loc, val);
    glCheckError();
}

void Shader::SetFloat2(const char* name, float val, float val2) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform2f(loc, val, val2);
    glCheckError();
}
void Shader::SetFloat2(const char* name, const float* val) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform2fv(loc, 1, val);
    glCheckError();
}

void Shader::SetFloat3(const char* name, float val, float val2, float val3) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform3f(loc, val, val2, val3);
    glCheckError();
}
void Shader::SetFloat3(const char* name, const float* val) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform3fv(loc, 1, val);
    glCheckError();
}

void Shader::SetFloat4(const char* name, float val, float val2, float val3, float val4) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform4f(loc, val, val2, val3, val4);
    glCheckError();
}
void Shader::SetFloat4(const char* name, const float* val) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform4fv(loc, 1, val);
    glCheckError();
}




void Shader::SetMat3(const char* name, const float* data, bool transpose) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniformMatrix3fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, data);
    glCheckError();
}
void Shader::SetMat4(const char* name, const float* data, bool transpose) {
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniformMatrix4fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, data);
    glCheckError();
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

static bool ParseShader(const char* const filePath, std::string outShaders[])
{
    std::ifstream file;
    file.open(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string* pCurrent = nullptr;

    while (!file.eof())
    {
        char buff[300];
        file.getline(buff, 300);

        if (!mystrcmp(buff, "#shader", 7))
        {
            //change type
            char* name = &buff[8];
            if (!mystrcmp(name, "vert", 4)) 
            {
                pCurrent = &outShaders[0];
            }
            else if (!mystrcmp(name, "frag", 4))
            {
                pCurrent = &outShaders[1];

            }
            else if (!mystrcmp(name, "geo", 3))
            {
                pCurrent = &outShaders[2];
            }
            else
            {
                LogWarn("Unknown shader type: %s", name);
            }
            
            if (pCurrent && pCurrent->empty()) {
                pCurrent->reserve(2000);
            }
        }
        else
        {
            // Assert(pCurrent && "Did not find #shader in .prog file");
            if (pCurrent)
            {
                pCurrent->append(buff);
                pCurrent->push_back('\n');
            }
        }
    }

    file.close();

    return true;
}

static const char* GetShaderName(unsigned int n) {
    switch (n)
    {
        case GL_VERTEX_SHADER: return "Vertex";
        case GL_FRAGMENT_SHADER: return "Fragment";
        case GL_GEOMETRY_SHADER: return "Geometry";
        default: return "(unknown)";
    }
}
static unsigned int CompileShader(unsigned int nShaderType, const char* strShaderCode) {
    unsigned int nId = glCreateShader (nShaderType);
    glShaderSource (nId, 1, &strShaderCode, 0);
    glCompileShader(nId);
    glCheckError();

    
    
    int nStatus = 0;
    glGetShaderiv (nId, GL_COMPILE_STATUS, &nStatus);
    glCheckError();
    if (!nStatus)
    {
        const char* name = GetShaderName(nShaderType);
        LogWarn ("Error: Failed to compile %s shader", name);
        int size;
        glGetShaderiv (nId, GL_INFO_LOG_LENGTH, &size);
        char* buffer = new(std::nothrow) char[size];
        if (buffer)
        {
            glGetShaderInfoLog (nId, size, nullptr, buffer);
            LogError ("Error: Failed to compile %s shader: %s", name, buffer);
            delete[] buffer;

        }
        Assert(false);
    }
    return nId;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

int mystrlen(const char* str)
{
    int i = 0;
    for (; str[i] != '\0'; i++);
    return i;
}

bool mystrcmp(const char* strA, const char* strB, int len)
{
    //return true if they are different strings
    int lenA = mystrlen(strA);
    int lenB = mystrlen(strB);

    if (len == -1)
    {
        if (lenA != lenB)	return true;
    }
    else
    {
        if (lenA < len || lenB < len) return true;
    }

    if (len == -1 || len > lenA)	len = lenA;

    for (int i = 0; i < len; i++)
    {
        if (strA[i] != strB[i])
            return true;
    }
    return false;
}

bool mystrcmpi(const char* strA, const char* strB, int len)
{
    //returns true when the strings are differernt
    int lenA = mystrlen(strA);
    int lenB = mystrlen(strB);

    if (len < 0)
    {
        if (lenA != lenB)   return true;

        len = lenA;
    }
    else {
        len = std::min(len, lenA);
        len = std::min(len, lenB);
    
    }

    for (int i = 0; i < len; i++)
    {
        if (tolower(strA[i]) != tolower(strB[i]))
        {
            return true;
        }
    }
    return false;
}