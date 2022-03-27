#pragma once
#include <GL/glew.h>
#include "DebugFinal.h"
#include <string>
#include <unordered_map>

int mystrlen(const char* str);
bool mystrcmp(const char* strA, const char* strB, int len);
bool mystrcmpi(const char* strA, const char* strB, int len);


class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const char* path);
    bool Load(const char* path);

    void Bind() const;
    void Unbind() const;

    unsigned int id() const { return m_id; }

    //Set uniforms
    void SetInt(const char* name, int val);
    void SetInts(const char* name, int* data, int count);

    void SetFloat(const char* name, float val);
    void SetFloat2(const char* name, float val, float val2);
    void SetFloat2(const char* name, const float* val);
    void SetFloat3(const char* name, float val, float val2, float val3);
    void SetFloat3(const char* name, const float* val);
    void SetFloat4(const char* name, float val, float val2, float val3, float val4);
    void SetFloat4(const char* name, const float* val);



    void SetMat3(const char* name, const float* data, bool transpose=false);
    void SetMat4(const char* name, const float* data, bool transpose=false);

private:
    int GetUniformLocation(const char* name) const;

private:
    unsigned int m_id;
    mutable std::unordered_map<std::string, int> mapUniforms;
};

