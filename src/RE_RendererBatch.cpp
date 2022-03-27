#include "RE_RendererBatch.h"
#include <type_traits>

RendererBatch::RendererBatch() :
    myBuffer(nullptr),
    myBufferSize(0),
    // myBufferPos(0),
    myIndexBuffer(nullptr),
    myIndexBufferLength(0),
    // myIndexBufferPos(0),

    myVertexCount(0),
    myIndexCount(0),
    myPrim(RE_PRIM_NONE),

    myMetLineDrawCalls(0),
    myMetLinePrimitives(0),
    myMetPointDrawCalls(0),
    myMetPointPrimitives(0),
    myMetTriDrawCalls(0),
    myMetTriPrimitives(0)

{

}
RendererBatch::~RendererBatch() {
    Cleanup();
}

void RendererBatch::Init(Camera* cam) {
    Renderer::Init(cam);

    constexpr uint64 BufferSize = 10'000 * sizeof(VertexTri);
    constexpr uint64 IndexBufferCount = 10'000;

    myBuffer = (uint8*)malloc(sizeof(uint8) * BufferSize);
    myBufferSize = BufferSize;
    // myBufferPos = 0;

    myIndexBuffer = (uint32*)malloc(sizeof(uint32) * IndexBufferCount);
    myIndexBufferLength = IndexBufferCount;
    // myIndexBufferPos = 0;

    myVbo.Init(BufferSize, nullptr);
    myIbo.Init(IndexBufferCount, nullptr);

    myVaoLine.Init();
    myVbo.SetLayout({
        { "Position", SType::Float3 },
        { "Col", SType::Float4 },
        { "Width", SType::Float },
    });
    bool bStatus = myShaderLine.Load("Assets/Shaders/line.prog");
    Assert(bStatus);

    myVaoPoints.Init();
    myVbo.SetLayout({
        { "Position", SType::Float3 },
        { "Col", SType::Float4 },
        { "Width", SType::Float },
    });
    bStatus = myShaderPoints.Load("Assets/Shaders/point.prog");
    Assert(bStatus);

    myVaoTri.Init();
    myVbo.SetLayout({
        { "Position", SType::Float3 },
        { "Col", SType::Float4 },
        { "Normal", SType::Float3 },
    });
    bStatus = myShaderTri.Load("Assets/Shaders/tri.prog");
    Assert(bStatus);

}
void RendererBatch::Cleanup() {
    if (myBuffer) {
        free(myBuffer);
        myBuffer = 0;
    }
    if (myIndexBuffer) {
        free(myIndexBuffer);
        myIndexBuffer = 0;
    }
}

template<typename T>
void RendererBatch::VBPush(const T& other) {
    static_assert(std::is_same<T, VertexPoint>::value || std::is_same<T, VertexLine>::value || std::is_same<T, VertexTri>::value, "Template type must be a Vertex");
    Assert(
        (std::is_same<T, VertexPoint>::value && myPrim == RE_PRIM_POINT) ||
        (std::is_same<T, VertexLine>::value  && myPrim == RE_PRIM_LINE ) ||
        (std::is_same<T, VertexTri>::value   && myPrim == RE_PRIM_TRI  ) ||
        !"Pushed the wrong type of vertex when the RendererBatch is in a different primitive mode"
    );

    Assert(VBHasSpace<T>(1) && "Buffer is full");
    T* pObj = (T*)(myBuffer + myVertexCount * sizeof(T));
    *pObj = other;
    myVertexCount++;
}

template<typename T>
T* RendererBatch::VBPush() {
    static_assert(std::is_same<T, VertexPoint>::value || std::is_same<T, VertexLine>::value || std::is_same<T, VertexTri>::value, "Template type must be a Vertex");
    Assert(
        (std::is_same<T, VertexPoint>::value && myPrim == RE_PRIM_POINT) ||
        (std::is_same<T, VertexLine>::value  && myPrim == RE_PRIM_LINE ) ||
        (std::is_same<T, VertexTri>::value   && myPrim == RE_PRIM_TRI  ) ||
        !"Pushed the wrong type of vertex when the RendererBatch is in a different primitive mode"
    );

    Assert(VBHasSpace<T>(1) && "Buffer is full");
    T* pObj = (T*)(myBuffer + myVertexCount * sizeof(T));
    myVertexCount++;
    return pObj;
}

template<typename T>
bool RendererBatch::VBHasSpace(uint64 count) {
    static_assert(std::is_same<T, VertexPoint>::value || std::is_same<T, VertexLine>::value || std::is_same<T, VertexTri>::value, "Template type must be a Vertex");
    Assert(
        (std::is_same<T, VertexPoint>::value && myPrim == RE_PRIM_POINT) ||
        (std::is_same<T, VertexLine>::value  && myPrim == RE_PRIM_LINE ) ||
        (std::is_same<T, VertexTri>::value   && myPrim == RE_PRIM_TRI  ) ||
        !"Checking space of the wrong type of vertex when the RendererBatch is in a different primitive mode"
    );

    // int a = (myVertexCount+count);
    // int b = sizeof(T);
    // int c = myBufferSize;
    // LogInfo("%d, %d, (%d) <= %d", a, b, a*b, c);

    return ( (myVertexCount+count) * sizeof(T) <= myBufferSize);
}

void RendererBatch::IBPush(int32 val) {
    Assert(IBHasSpace(1) && "Index Buffer is full");
    myIndexBuffer[myIndexCount] = val;
    ++myIndexCount;
}
void RendererBatch::IBPush(int32* ar, uint32 count) {
    Assert(IBHasSpace(count) && "Index Buffer is full");
    for (uint32 i = 0; i < count; i++) {
        myIndexBuffer[myIndexCount] = ar[i];
        ++myIndexCount;
    }
}
bool RendererBatch::IBHasSpace(uint32 count) {
    return (myIndexCount+count <= myIndexBufferLength); 
}

void RendererBatch::StartFrame() {
    myVertexCount = 0;
    myIndexCount = 0;

    //Metrics
    myMetLineDrawCalls = 0;
    myMetLinePrimitives = 0;
    myMetPointDrawCalls = 0;
    myMetPointPrimitives = 0;
    myMetTriDrawCalls = 0;
    myMetTriPrimitives = 0;
}

void RendererBatch::EndFrame() {
    Flush();
}

void RendererBatch::SwitchPrim(Primitive p)
{
    if (myPrim != p) {
        Flush();
        myPrim = p;
    }
}

void RendererBatch::PrintMetrics() {
    uint64 total = myMetLineDrawCalls + myMetPointDrawCalls + myMetTriDrawCalls;

    LogL(LOG_LEVEL_INFO, LOG_ENDL);
    // LogL(LOG_LEVEL_INFO, "%s" LOG_ENDL, LOG_COL_INFO);
    LogL(LOG_LEVEL_INFO, "---------    RendererBatch Metrics    ------------" LOG_ENDL);
    LogL(LOG_LEVEL_INFO, "Line Draw Calls  : %s%03d%s    Lines : %s%07d%s" LOG_ENDL, LOG_COL_INFO, myMetLineDrawCalls, LOG_COL_RESET, LOG_COL_INFO, myMetLinePrimitives, LOG_COL_RESET);
    LogL(LOG_LEVEL_INFO, "Point Draw Calls : %s%03d%s    Points: %s%07d%s" LOG_ENDL, LOG_COL_INFO, myMetPointDrawCalls, LOG_COL_RESET, LOG_COL_INFO, myMetPointPrimitives, LOG_COL_RESET);
    LogL(LOG_LEVEL_INFO, "Tri   Draw Calls : %s%03d%s    Tri   : %s%07d%s" LOG_ENDL, LOG_COL_INFO, myMetTriDrawCalls, LOG_COL_RESET, LOG_COL_INFO, myMetTriPrimitives, LOG_COL_RESET);

    LogL(LOG_LEVEL_INFO, LOG_ENDL);
    LogL(LOG_LEVEL_INFO, "Total Draw Calls : %s%05d%s" LOG_ENDL, LOG_COL_INFO, total, LOG_COL_RESET);
    // LogL(LOG_LEVEL_INFO, "%s" LOG_ENDL, LOG_COL_RESET);
    LogL(LOG_LEVEL_INFO, LOG_ENDL);

}

void RendererBatch::DoFlush() {
    if (myPrim == RE_PRIM_NONE)
        return;
    
    if (myPrim == RE_PRIM_POINT)
    {
        if (myVertexCount == 0)
            return;
        myVaoPoints.Bind();
        myShaderPoints.Bind();
        glm::mat4 mat = glm::mat4(1.0f);
        mat = myCam->VP();
        myShaderPoints.SetMat4("mat_proj", glm::value_ptr(mat));
        myVbo.Bind();
        myVbo.Update(0, myBuffer, myVertexCount*sizeof(VertexPoint));

        // glPointSize(15);
        glCheckError();
        glDrawArrays(GL_POINTS, 0, myVertexCount);
        glCheckError();

        myMetPointDrawCalls++;
    }
    else if (myPrim == RE_PRIM_LINE)
    {
        if (myVertexCount == 0 || myIndexCount == 0) {
            Assert(myVertexCount == 0 && myIndexCount == 0);
            return;
        }
        myVaoLine.Bind();
        myShaderLine.Bind();
        glm::mat4 mat = glm::mat4(1.0f);
        mat = myCam->VP();
        myShaderLine.SetMat4("mat_proj", glm::value_ptr(mat));
        myVbo.Bind();
        myVbo.Update(0, myBuffer, myVertexCount*sizeof(VertexLine));
        myIbo.Bind();
        myIbo.Update(0, myIndexBuffer, myIndexCount);

        glCheckError();
        glDrawElements(GL_LINES, myIndexCount, GL_UNSIGNED_INT, 0);
        glCheckError();

        myMetLineDrawCalls++;
    }
    else if (myPrim == RE_PRIM_TRI)
    {
        if (myVertexCount == 0 || myIndexCount == 0) {
            Assert(myVertexCount == 0 && myIndexCount == 0);
            return;
        }

        myVaoTri.Bind();
        myShaderTri.Bind();
        glm::mat4 mat = glm::mat4(1.0f);
        mat = myCam->VP();
        myShaderTri.SetMat4("mat_proj", glm::value_ptr(mat));

        //Light uniforms
        float ambientStrength = 0.4;
        glm::vec3 lightCol = glm::vec3 (1.0f, 1.0f, 220.0 / 255.0);
        glm::vec3 lightDir = glm::vec3 (-1, -1, -1);

        myShaderTri.SetFloat("ambient_strength", ambientStrength);
        myShaderTri.SetFloat3("light_color", glm::value_ptr(lightCol));
        myShaderTri.SetFloat3("light_dir", glm::value_ptr(lightDir));

        myVbo.Bind();
        myVbo.Update(0, myBuffer, myVertexCount*sizeof(VertexTri));
        myIbo.Bind();
        myIbo.Update(0, myIndexBuffer, myIndexCount);

        glCheckError();
        glDrawElements(GL_TRIANGLES, myIndexCount, GL_UNSIGNED_INT, 0);
        glCheckError();

        myMetTriDrawCalls++;
    }

    VBClear();
    IBClear();
    glCheckError();

}

void RendererBatch::DrawPoint(glm::vec3 pos, glm::vec4 col, float width)
{
    SwitchPrim(RE_PRIM_POINT);
    if (!VBHasSpace<VertexPoint>(1)) {
        Flush();
    }
    Assert( VBHasSpace<VertexPoint>(1) );

    VertexPoint* pVertex = VBPush<VertexPoint>();
    pVertex->Pos = pos;
    pVertex->Col = col;
    pVertex->Width = width;

    myFlush = true;
    //Metrics
    myMetPointPrimitives++;
}

void RendererBatch::DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec4 col, float width)
{
    SwitchPrim(RE_PRIM_LINE);
    if (!VBHasSpace<VertexLine>(2) || !IBHasSpace(2)) {
        Flush();
    }

    Assert(VBHasSpace<VertexLine>(2) && "Flush didnt create enough space... Buffer needs to be bigger");
    Assert(IBHasSpace(2) && "Flush didnt create enough space... Buffer needs to be bigger");

    uint32 vertexCount = myVertexCount;

    VertexLine* pv = VBPush<VertexLine>();
    pv->Pos = p1;
    pv->Col = col;
    pv->Width = width;

    pv = VBPush<VertexLine>();
    pv->Pos = p2;
    pv->Col = col;
    pv->Width = width;

    IBPush(vertexCount);
    IBPush(vertexCount+1);

    myFlush = true;
    //Metric
    myMetLinePrimitives++;
}

void RendererBatch::DrawLineStrip(const glm::vec3* pos, int32 count, glm::vec4 col, float width)
{
    if (count <= 1) {
        LogWarn("Cannot draw LineStrip with count: %d", count);
        return;
    }
    
    SwitchPrim(RE_PRIM_LINE);
    if (!VBHasSpace<VertexLine>(count) || !IBHasSpace( 2*(count-1) )) {
        Flush();
        if (!VBHasSpace<VertexLine>(count) || !IBHasSpace( 2*(count-1) )) {
            //Flush did not create enough space. This is because the buffer is not big enough to render it in a single draw call.
            //In this case we split up the line strip into smaller line strips
            // Assert(false && "Todo: implement this");
            
            int32 newCount = (count+1)/2;
            //While splitting into smaller regions, we should never split too small that we cant render anymore. If this happens then increase the buffer size
            Assert(newCount >= 2);
            Assert( (count-newCount+1) >= 2);

            DrawLineStrip(&pos[0], newCount, col, width);
            DrawLineStrip(&pos[newCount-1], count-newCount+1, col, width);
            return;
        }
    }

    uint32 vertexCount = myVertexCount;
    VertexLine* pv = VBPush<VertexLine>();
    pv->Pos = pos[0];
    pv->Col = col;
    pv->Width = width;

    for (int i = 1; i < count; i++) {
        pv = VBPush<VertexLine>();
        pv->Pos = pos[i];
        pv->Col = col;
        pv->Width = width;

        IBPush(vertexCount + (i-1));    //Previous vertex
        IBPush(vertexCount + i);        //Cur Vertex
    }

    myFlush = true;
    //Metric
    myMetLinePrimitives += count-1;
}

void RendererBatch::DrawLineLoop(const glm::vec3* pos, int32 count, glm::vec4 col, float width)
{
    if (count <= 1) {
        LogWarn("Cannot draw LineLoop with count: %d", count);
        return;
    }
    
    SwitchPrim(RE_PRIM_LINE);
    if (!VBHasSpace<VertexLine>(count) || !IBHasSpace(2*count)) {
        Flush();
        if (!VBHasSpace<VertexLine>(count) || !IBHasSpace(2*count)) {
            //Flush did not create enough space. This is because the buffer is not big enough to render it in a single draw call.
            //In this case we split up the line loop into a line strip and manually draw the last line
            DrawLineStrip(pos, count, col, width);
            DrawLine(pos[0], pos[count-1], col, width);
            return;
        }
    }
    
    uint32 vertexCount = myVertexCount;
    VertexLine* pv = VBPush<VertexLine>();
    pv->Pos = pos[0];
    pv->Col = col;
    pv->Width = width;

    for (int i = 1; i < count; i++) {
        pv = VBPush<VertexLine>();
        pv->Pos = pos[i];
        pv->Col = col;
        pv->Width = width;

        IBPush(vertexCount + (i-1));    //Previous vertex
        IBPush(vertexCount + i);        //Cur Vertex
    }
    //For line loop connect the last vertex with the first vertex
    IBPush(vertexCount + (count-1));
    IBPush(vertexCount);

    myFlush = true;
    //Metric
    myMetLinePrimitives += count;
}

glm::vec3 CalculateNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    const glm::vec3 x = b-a;
    const glm::vec3 y = c-a;
    return glm::normalize( glm::cross(x, y) );
}
void RendererBatch::DrawTriangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3, glm::vec4 col)
{
    SwitchPrim(RE_PRIM_TRI);
    if (!VBHasSpace<VertexTri>(3) || !IBHasSpace(3)) {
        Flush();
        Assert(VBHasSpace<VertexTri>(3) && IBHasSpace(3) && "Buffer is not big enough");
    }
    uint32 vertexCount = myVertexCount;
    glm::vec3 normal = CalculateNormal(pos1, pos2, pos3);

    VertexTri* pv = VBPush<VertexTri>();
    pv->Pos = pos1;
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;
    pv->Normal = normal;

    pv = VBPush<VertexTri>();
    pv->Pos = pos2;
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;
    pv->Normal = normal;

    pv = VBPush<VertexTri>();
    pv->Pos = pos3;
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;
    pv->Normal = normal;

    IBPush(vertexCount);
    IBPush(vertexCount+1);
    IBPush(vertexCount+2);

    myFlush = true;
    //Metric
    myMetTriPrimitives++;

    //float s = 1.0f;
    //glm::vec4 col2 = glm::vec4(0.7, 0.2, 0.2, 1.0);
    //DrawLine(pos1, pos1 + normal*s, col2, 1);
    //DrawLine(pos2, pos2 + normal*s, col2, 1);
    //DrawLine(pos3, pos3 + normal*s, col2, 1);
    //Flush();

}

void RendererBatch::DrawTriangleStrip(const glm::vec3* pos, const int32 count, glm::vec4 col, bool bFlipNormal)
{
    SwitchPrim(RE_PRIM_TRI);
    if (count <= 2) {
        LogWarn("Cannot draw Triangle Strip with count: %d", count);
        return;
    }
    const int32 newIndicesCount = (count-2) * 3;
    if (!VBHasSpace<VertexTri>(count) || !IBHasSpace(newIndicesCount)) {
        Flush();
        if (!VBHasSpace<VertexTri>(count) || !IBHasSpace(newIndicesCount))
        {
            //Buffer isnt big enough to render in a single draw call.. Need to do some recursion
            const int32 newTriCount = (count-2) / 2;
            const int32 newCount = newTriCount + 2;
            const int32 newCount2 = count - newCount + 2;
            //While splitting into smaller regions, we should never split too small that we cant render anymore. If this happens then increase the buffer size
            Assert( newCount >= 3 );
            Assert( newCount2 >= 3 );
            DrawTriangleStrip(&pos[0], newCount, col);
            DrawTriangleStrip(&pos[newCount-2], newCount2, col);
            return;
        }
    }

    uint32 vertexCount = myVertexCount;
    //We can treat this as an array
    VertexTri* pStartVertex = VBPush<VertexTri>();  

    VertexTri* pv = pStartVertex;
    pv->Pos = pos[0];
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;
    pv->Normal = glm::vec3(0.0f, 0.0f, 0.0f);

    pv = VBPush<VertexTri>();
    pv->Pos = pos[1];
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;
    pv->Normal = glm::vec3(0.0f, 0.0f, 0.0f);

    for (int32 i = 2; i < count; i++) {
        pv = VBPush<VertexTri>();
        pv->Pos = pos[i];
        // pv->Col = col + glm::vec4(0.0, 1.0 * (float)i/count, 0.0, 0.0);
        pv->Col = col;
        //pv->Tex = {0.0f, 0.0f};
        //pv->TexId = -1;

        glm::vec3 n = CalculateNormal(pStartVertex[i-2].Pos, pStartVertex[i-1].Pos, pv->Pos);
        if ( (i + (int)bFlipNormal) % 2 == 0 ) {
            //I believe opengl flips the winding order every alternate triangle so that all triangles have the same winding order
            // As a result, even though the positions are in the 'wrong' order, we need to flip the normal to get the correct normal
            
            n *= -1.0f;
        }
        pv->Normal = n;
        pStartVertex[i-1].Normal += n;
        pStartVertex[i-2].Normal += n;

        IBPush(vertexCount + i-2);
        IBPush(vertexCount + i-1);
        IBPush(vertexCount + i);
    }

    glm::vec3 eps = glm::vec3(0.01);
    glm::vec3* pn = &pStartVertex[1].Normal;

    //If the vector is 0.0f, then don't attempt to normalize it
    if (!glm::all( glm::lessThan( glm::abs(*pn), eps )))
        *pn = glm::normalize( *pn / 2.0f );
    pn = &pStartVertex[count-2].Normal;
    if (!glm::all( glm::lessThan( glm::abs(*pn), eps )))
        *pn = glm::normalize( *pn / 2.0f );

    for (int32 i = 2; i < count-2; i++) {
        pn = &pStartVertex[i].Normal;
        if (!glm::all( glm::lessThan( glm::abs(*pn), eps )))
            *pn = glm::normalize( *pn / 3.0f );
    }

    myFlush = true;
    //Metric
    myMetTriPrimitives += (count-2);



#if 0
    glm::vec3* posss    = new glm::vec3[count];
    glm::vec3* normalss = new glm::vec3[count];

    for (int i = 0; i < count; i++) {
        posss[i] = pStartVertex[i].Pos;
        normalss[i] = pStartVertex[i].Normal;
    }

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    DoFlush();
    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    PushDepthState(RE_DEPTH_LESS);
    for (int32 i = 0 ; i < count; i++) {
        DrawLine( posss[i], posss[i] + normalss[i] * 0.3f, glm::vec4(0.8f, 0.2f, 0.2f, 1.0f), 2);
    }
    PopDepthState();
    delete[] posss;
    delete[] normalss;
    glCheckError();
    DoFlush();
#endif

}


void RendererBatch::DrawTriangleFanPrivate(glm::vec3 posBase, const glm::vec3* pos, int32 count, glm::vec4 col) {

    SwitchPrim(RE_PRIM_TRI);
    if (count <= 1) {
        LogWarn("Cannot draw Triangle Fan with count: %d", count);
        return;
    }

    const int32 newIndicesCount = (count-1) * 3;
    if (!VBHasSpace<VertexTri>(count+1) || !IBHasSpace(newIndicesCount)) {
        Flush();
        if (!VBHasSpace<VertexTri>(count+1) || !IBHasSpace(newIndicesCount))
        {
            //Buffer isnt big enough to render in a single draw call.. Need to do some recursion
            const int32 newTriCount = (count-1) / 2;
            const int32 newCount = newTriCount + 1;
            const int32 newCount2 = count - newCount + 1;
            //While splitting into smaller regions, we should never split too small that we cant render anymore. If this happens then increase the buffer size
            Assert(newCount >= 2);
            Assert(newCount2 >= 2);
            DrawTriangleFanPrivate(posBase, &pos[0], newCount, col);
            DrawTriangleFanPrivate(posBase, &pos[newCount-1], newCount2, col);
            return;
        }
    }

    uint32 vertexCount = myVertexCount;
    VertexTri* pv = VBPush<VertexTri>();
    pv->Pos = posBase;
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;

    pv = VBPush<VertexTri>();
    pv->Pos = pos[0];
    pv->Col = col;
    //pv->Tex = {0.0f, 0.0f};
    //pv->TexId = -1;

    for (int32 i = 1; i < count; i++) {
        pv = VBPush<VertexTri>();
        pv->Pos = pos[i];
        pv->Col = col;
        //pv->Tex = {0.0f, 0.0f};
        //pv->TexId = -1;

        IBPush(vertexCount);
        IBPush(vertexCount + i);
        IBPush(vertexCount + i+1);
    }

    myFlush = true;
    //Metric
    myMetTriPrimitives += (count-1);
}


