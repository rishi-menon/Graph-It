#pragma once
#include "DebugFinal.h"
#include "RE_RendererState.h"

#include "RE_Buffers.h"
#include "RE_Shader.h"
#include "RE_Texture.h"
#include "Camera.h"



class Renderer : public RendererState {
public:
    Renderer() = default;
    virtual ~Renderer();
    Renderer(const Renderer&) = delete;

    virtual void Init(Camera* cam);

    virtual void PrintMetrics() = 0;
    virtual void StartFrame() = 0;
    virtual void EndFrame() = 0;

    //Render related
    virtual void DrawPoint(glm::vec3 pos, glm::vec4 col, float width) = 0;

    virtual void DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec4 col, float width) = 0;
    virtual void DrawLineLoop(const glm::vec3* pos, int32 count, glm::vec4 col, float width) = 0;
    virtual void DrawLineStrip(const glm::vec3* pos, int32 count, glm::vec4 col, float width) = 0;

    virtual void DrawTriangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3, glm::vec4 col) = 0;

    //When bFlipNormal is false, clockwise order of vertices corresponds to a normal into the screen.
    //When true, clockwise order corresponds to normal out of the screen
    virtual void DrawTriangleStrip(const glm::vec3* pos, int32 count, glm::vec4 col, bool bFlipNormal = false) = 0;
    void DrawTriangleFan(const glm::vec3* pos, int32 count, glm::vec4 col) 
                    { DrawTriangleFanPrivate(pos[0], &pos[1], count-1, col); }

protected:
    virtual void DrawTriangleFanPrivate(glm::vec3 posBase, const glm::vec3* pos, int32 count, glm::vec4 col) = 0;

private:
    void Cleanup();

//Member variables
protected:
    Camera*             myCam = 0;

};

#include "RE_RendererBatch.h"