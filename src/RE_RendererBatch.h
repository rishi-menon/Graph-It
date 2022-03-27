#pragma once
#include "DebugFinal.h"
#include "RE_Renderer.h"

#include "RE_Buffers.h"
#include "RE_Shader.h"
#include "RE_Texture.h"
#include "Maths.h"
#include "Camera.h"

// extern glm::ivec2 windowSize;

struct VertexPoint {
    glm::vec3 Pos;
    glm::vec4 Col;
    float Width;
};
struct VertexLine {
    glm::vec3 Pos;
    glm::vec4 Col;
    float Width;
};
struct VertexTri {
    glm::vec3 Pos;
    glm::vec4 Col;
    glm::vec3 Normal;
};

class RendererBatch : public Renderer {
public:
    RendererBatch();
    ~RendererBatch();
    RendererBatch(const RendererBatch&) = delete;

    void Init(Camera* cam) override;
    
    void PrintMetrics() override;
    void DoFlush() override;
    void StartFrame() override;
    void EndFrame() override;

    //Render related
    void DrawPoint(glm::vec3 pos, glm::vec4 col, float width) override;

    void DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec4 col, float width) override;
    void DrawLineLoop(const glm::vec3* pos, int32 count, glm::vec4 col, float width) override;
    void DrawLineStrip(const glm::vec3* pos, int32 count, glm::vec4 col, float width) override;

    void DrawTriangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3, glm::vec4 col) override;
    void DrawTriangleStrip(const glm::vec3* pos, int32 count, glm::vec4 col, bool bFlipNormal = false) override;

private:
    enum Primitive {
        RE_PRIM_NONE,
        RE_PRIM_POINT,
        RE_PRIM_LINE,
        RE_PRIM_TRI,
    };
    void SwitchPrim(Primitive p);

    //Vertex Buffer. Template parameter should only be a vertex struct ideally
    template<typename T>
    void VBPush(const T& other);

    template<typename T>
    T* VBPush();
    
    template<typename T>
    bool VBHasSpace(uint64 count);

    inline void VBClear() { myVertexCount = 0; }

    void IBPush(int32 val);
    void IBPush(int32* ar, uint32 count);
    bool IBHasSpace(uint32 count);
    inline void IBClear() { myIndexCount = 0; }
   
    void Cleanup();

private:
    void DrawTriangleFanPrivate(glm::vec3 posBase, const glm::vec3* pos, int32 count, glm::vec4 col);

private:
    uint8*              myBuffer;
    uint64              myBufferSize;
    uint32              myVertexCount;

    uint32*             myIndexBuffer;
    uint64              myIndexBufferLength;
    uint32              myIndexCount;

    Primitive           myPrim;
    VertexBuffer        myVbo;
    IndexBuffer         myIbo;

    //Line stuff
    VertexArray         myVaoLine;
    Shader              myShaderLine;

    //Points stuff
    VertexArray         myVaoPoints;
    Shader              myShaderPoints;

    //Tri stuff
    VertexArray         myVaoTri;
    Shader              myShaderTri;
    
    //Metrics
    uint64               myMetLineDrawCalls;
    uint64               myMetLinePrimitives;
    uint64               myMetPointDrawCalls;
    uint64               myMetPointPrimitives;
    uint64               myMetTriDrawCalls;
    uint64               myMetTriPrimitives;
};
