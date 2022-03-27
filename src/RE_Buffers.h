#pragma once
#include <GL/glew.h>
#include <vector>
#include "DebugFinal.h"
#include "RE_RendererState.h"

constexpr uint InvalidId = (uint)(-1);

enum class SType {
    Float = 0,
    Float2,
    Float3,
    Float4,

    Int
};

struct AttribType {
    AttribType() = default;
    AttribType(const char* name, SType type) :
        Name(name), Type(type), Normalized(false), Offset(0)
    {
    }

    AttribType(const char* name, SType type, uint64 off) :
        Name(name), Type(type), Normalized(false), Offset(off)
    {
    }

    AttribType(const char* name, SType type, bool norm, uint64 off) :
        Name(name), Type(type), Normalized(norm), Offset(off)
    {
    }

    const char* Name;
    SType Type;
    bool Normalized;
    uint64 Offset;  //This is because structs might have padding which messes the calculation
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;
    uint id() const { return m_id; }
    void Init();

private:
    const static VertexArray* s_pBound;
private:
    uint m_id;

};

class VertexBuffer {
public:
    VertexBuffer();
    ~VertexBuffer();
    
    void Bind() const;
    void Unbind() const;
    uint id() const { return m_id; }

    void Init(int32 size, const void* data, uint32 usage = GL_DYNAMIC_DRAW);
    void SetLayout(const std::vector<AttribType>& arrTypes, int32 size = 0);
    
    void Update(int32 offset, const void* data, int32 size);

private:
    const static VertexBuffer* s_pBound;
private:
    uint m_id;

#ifdef RM_DEBUG
    int32 m_size;
#endif

};

class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer();
    void Bind() const;
    void Unbind() const;
    uint id() const { return m_id; }
    
    void Init(int32 count, const uint32* data, uint32 usage = GL_DYNAMIC_DRAW);
    void Update(int32 offset, const uint32* data, int32 count);

private:
    const static IndexBuffer* s_pBound;
private:
    uint m_id;

#ifdef RM_DEBUG
    int32 m_size;
#endif
};