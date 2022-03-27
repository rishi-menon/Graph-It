#include "RE_Buffers.h"

const VertexArray*  VertexArray::s_pBound = nullptr;
const VertexBuffer* VertexBuffer::s_pBound = nullptr;
const IndexBuffer*  IndexBuffer::s_pBound = nullptr;

static uint32 SizeFromSType (SType t) {
    switch (t) {
        case SType::Float:   return 1*sizeof(float);
        case SType::Float2:  return 2*sizeof(float);
        case SType::Float3:  return 3*sizeof(float);
        case SType::Float4:  return 4*sizeof(float);

        case SType::Int:     return 1*sizeof(int);
    }
    Assert(!"Unknown Type");
    return 0;
}
static uint32 CountFromSType (SType t) {
    switch (t) {
        case SType::Float:   return 1;
        case SType::Float2:  return 2;
        case SType::Float3:  return 3;
        case SType::Float4:  return 4;

        case SType::Int:     return 1;
    }
    Assert(!"Unknown Type");
    return 0;
}
static uint32 TypeFromSType (SType t) {
    switch (t) {
        case SType::Float:   return GL_FLOAT;
        case SType::Float2:  return GL_FLOAT;
        case SType::Float3:  return GL_FLOAT;
        case SType::Float4:  return GL_FLOAT;

        case SType::Int:     return GL_INT;
    }
    Assert(!"Unknown Type");
    return 0;
}

////////////////////////////////////////////
//////          Vertex Array
////////////////////////////////////////////
VertexArray::VertexArray() {
    m_id = InvalidId;
}
VertexArray::~VertexArray() {
    if (m_id != InvalidId)
    {
        glDeleteVertexArrays(1, &m_id);
        if (s_pBound == this)
            s_pBound = nullptr;
    }
}
void VertexArray::Init() {
    Assert(m_id == InvalidId && "Already initialised");
    for (int tries = 0; tries < 100 && m_id == InvalidId; tries++) {
        glGenVertexArrays(1, &m_id);
    }
    Bind();
    glCheckError();
    Assert(m_id != InvalidId && "Failed to initialize");
}

void VertexArray::Bind() const {
    Assert(m_id != InvalidId && "Failed to initialize");
    glBindVertexArray(m_id);
    s_pBound = this;
}
void VertexArray::Unbind() const {
    Assert(m_id != InvalidId && "Failed to initialize");
    if (s_pBound != this) {
        LogWarn("Unbinding the wrong vertex array");
        return;
    }
    glBindVertexArray(0);
    s_pBound = nullptr;
}

////////////////////////////////////////////
//////          Vertex Buffer
////////////////////////////////////////////
VertexBuffer::VertexBuffer()
{
    m_id = InvalidId;
}
VertexBuffer::~VertexBuffer() {
    if (m_id != InvalidId)
    {
        glDeleteBuffers(1, &m_id);
        if (s_pBound == this) {
            s_pBound = nullptr;
        }
    }
}
void VertexBuffer::Bind() const {
    Assert(m_id != InvalidId && "Failed to initialize");
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    s_pBound = this;
}
void VertexBuffer::Unbind() const {
    Assert(m_id != InvalidId && "Failed to initialize");
    if (s_pBound != this) {
        LogWarn("Unbinding the wrong vertex buffer");
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    s_pBound = nullptr;
}

void VertexBuffer::Init(int32 size, const void* data, uint32 usage) {
    if (m_id != InvalidId) {
        LogError("Vertex buffer has already been initialised");
        return;
    }
    
    #ifdef RM_DEBUG
        m_size = size;
    #endif

    for (int tries = 0; tries < 100 && m_id == InvalidId; tries++) {
        glGenBuffers(1, &m_id);
    }
    Bind();
    glCheckError();
    Assert(m_id != InvalidId && "Failed to initialize");

    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    glCheckError();
}
void VertexBuffer::Update(int32 offset, const void* data, int32 size) {
    Assert(m_id != InvalidId && "Failed to initialize");
    Assert(s_pBound==this);
    #ifdef RM_DEBUG
        Assert(offset+size <= m_size && "Buffer overflow");
    #endif
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    glCheckError();
}

void VertexBuffer::SetLayout(const std::vector<AttribType>& arrTypes, int32 size) {
    Assert(m_id != InvalidId && "Failed to initialize");
    Assert(s_pBound==this);
    uint32 nSize;
    if (size > 0)
        nSize = size;
    else {
        nSize = 0;
        for (uint32 i = 0; i < arrTypes.size(); i++) {
            const AttribType& t = arrTypes.at(i);
            // LogInfo("Name: %s, Type: %d, Norm: %d, off: %d", t.Name, t.Type, t.Normalized, nSize);
            nSize += SizeFromSType(t.Type);
        }
    }
    // LogInfo("Size: %d", nSize);

    uint64 nOffset = 0;
    for (uint32 i = 0; i < arrTypes.size(); i++) {
        const AttribType& t = arrTypes.at(i);
        glEnableVertexAttribArray(i);
        if (t.Offset)
        {
            nOffset = t.Offset; 
            LogInfo("Name: %s, Type: %d, Norm: %d, off: %d", t.Name, t.Type, t.Normalized, t.Offset);
        }
        Assert(nOffset < nSize && "Something went wrong. You probably forgot to pass the size as the second paramter while using custom offsets");
        glVertexAttribPointer(i, CountFromSType(t.Type), TypeFromSType(t.Type), t.Normalized, nSize, (const void*)nOffset);
        nOffset += SizeFromSType(t.Type);
    }
}


////////////////////////////////////////////
//////          Index Buffer
////////////////////////////////////////////
IndexBuffer::IndexBuffer()
{
    m_id = InvalidId;
}
IndexBuffer::~IndexBuffer() {
    if (m_id != InvalidId)
    {
        glDeleteBuffers(1, &m_id);
        if (s_pBound == this) {
            s_pBound = nullptr;
        }
    }
}
void IndexBuffer::Bind() const {
    Assert(m_id != InvalidId && "Failed to initialise");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    s_pBound = this;
}
void IndexBuffer::Unbind() const {
    Assert(m_id != InvalidId && "Failed to initialise");
    if (s_pBound != this) {
        LogWarn("Unbinding the wrong index buffer");
        return;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    s_pBound = nullptr;
}

void IndexBuffer::Init(int32 count, const uint32* data, uint32 usage) {
    if (m_id != InvalidId) {
        LogError("Vertex buffer has already been initialised");
        return;
    }
    #ifdef RM_DEBUG
        m_size = sizeof(uint32) * count;
    #endif
    for (int tries = 0; tries < 100 && m_id == InvalidId; tries++)
    {
        glGenBuffers(1, &m_id);
    }
    Bind();
    glCheckError();
    Assert(m_id != InvalidId && "Failed to initialise");

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * count, data, usage);
    glCheckError();
}

void IndexBuffer::Update(int32 offset, const uint32* data, int32 count) {
    Assert(m_id != InvalidId && "Failed to initialise");
    Assert(s_pBound==this);
    #ifdef RM_DEBUG
        Assert( (offset+count) * sizeof(uint32) <= m_size && "Buffer overflow");
    #endif

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * offset, sizeof(uint32) * count, data);
    glCheckError();
}
