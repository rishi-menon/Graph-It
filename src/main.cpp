
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <GL/glew.h>

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <new>

#include "EventCallback.h"
#include "DebugFinal.h"
#include "Maths.h"

#include "RE_Buffers.h"
#include "RE_Texture.h"
#include "RE_Shader.h"
#include "RE_Renderer.h"
#include "RE_Font.h"

#include "Camera.h"
#include "Grapher3D.h"

#include "Maths.h"
#include "MathContext.h"
#include <fstream>

#ifdef _WIN32
#include "Windows.h"
#endif

//This is very temporary... Should not have a global camera or renderer. This is currently used by EventCallback
Camera* g_cam;
Renderer* g_renderer;
const char* g_strEqFile = nullptr;
MathParser::Context* g_ctx = nullptr;
bool g_updateGrapher = true;

double func(double x, double y) {
    return sin(x) * exp(y/7);
}
double funcImplicit(double x, double y, double z) {
    return z - y - sin(x);
}


//#define CATCH_SIGINT 1
volatile bool bRunning = true;

static void glfw_error_callback(int error, const char* description) {
    LogError("Glfw Error %d: %s", error, description);
}

#if CATCH_SIGINT
void signal_handler(int) {
    bRunning = false;
}
#endif


int g_OldViewport[4] = { 0, 0, 0, 0 };
void ResetViewport() {
    if (g_OldViewport[2] != 0 && g_OldViewport[3] != 0) {
        glViewport(g_OldViewport[0], g_OldViewport[1], g_OldViewport[2], g_OldViewport[3]);
    }
}
void viewport(int x, int y, int w, int h) {
    glViewport(x, y, w, h);
    
    g_OldViewport[0] = x;
    g_OldViewport[1] = y;
    g_OldViewport[2] = w;
    g_OldViewport[3] = h;
}

glm::ivec2 windowSize = { 1800, 1200 };
double deltaTime;


void DrawGrid(Renderer* r) {
    double majInc = 1;
    glm::vec4 majCol = glm::vec4(0.2, 0.2, 0.2, 1.0);
    double majWidth = 1.0;

    glm::vec4 axisXCol = glm::vec4(0.8, 0.2, 0.2, 1.0);
    glm::vec4 axisYCol = glm::vec4(0.2, 0.8, 0.2, 1.0);
    glm::vec4 axisZCol = glm::vec4(0.2, 0.2, 0.8, 1.0);


    //Todo: This is a temporary fix to prevent grid from showing above the graph. Have a more permanent fix than this
    glDepthMask(false);

    double lastLine = 20;
    double eps = 0.001;
    r->PushDepthState(RE_DEPTH_LESS);
    for(double x = 0; x < lastLine + eps; x += majInc) {
        r->DrawLine(glm::vec3(x, -lastLine, 0), glm::vec3(x, +lastLine, 0), majCol, majWidth);
    }
    for(double x = 0; x > -lastLine - eps; x -= majInc) {
        r->DrawLine(glm::vec3(x, -lastLine, 0), glm::vec3(x, +lastLine, 0), majCol, majWidth);
    }

    for(double y = 0; y < lastLine + eps; y += majInc) {
        r->DrawLine(glm::vec3(-lastLine, y, 0), glm::vec3(+lastLine, y, 0), majCol, majWidth);
    }
    for(double y = 0; y > -lastLine - eps; y -= majInc) {
        r->DrawLine(glm::vec3(-lastLine, y, 0), glm::vec3(+lastLine, y, 0), majCol, majWidth);
    }

    r->SetDepthState(RE_DEPTH_ALWAYS);
    //Draw xyz axes in a different colour
    r->DrawLine(glm::vec3(0, 0, 0), glm::vec3(lastLine, 0, 0), axisXCol, majWidth);
    r->DrawLine(glm::vec3(0, 0, 0), glm::vec3(0, lastLine, 0), axisYCol, majWidth);
    r->DrawLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, lastLine), axisZCol, majWidth);
    r->PopDepthState();

    //Todo: This is a temporary fix to prevent grid from showing above the graph. Have a more permanent fix than this
    glDepthMask(true);
}

void Print(const glm::vec4& v) {
    LogInfo("%+.02f, %+.02f, %+.02f, %+.02f", v.x, v.y, v.z, v.a);
}
void Print(const glm::vec3& v) {
    LogInfo("%+.02f, %+.02f, %+.02f", v.x, v.y, v.z);
}

void Print(const glm::mat4& m) {
    LogInfo("%+.02f, %+.02f, %+.02f, %+.02f", m[0][0], m[0][1], m[0][2], m[0][3]);
    LogInfo("%+.02f, %+.02f, %+.02f, %+.02f", m[1][0], m[1][1], m[1][2], m[1][3]);
    LogInfo("%+.02f, %+.02f, %+.02f, %+.02f", m[2][0], m[2][1], m[2][2], m[2][3]);
    LogInfo("%+.02f, %+.02f, %+.02f, %+.02f", m[3][0], m[3][1], m[3][2], m[3][3]);
}

void Debug() {
    Assert(MathParser::Context::RunAllTests() && "A test failed");

    MathParser::Context c;

    c.LoadFromFile("equations.txt");
    c.PrintProperties(true);
    
}

void UpdateGraphers(std::vector<Grapher3D>& graphers, MathParser::Context& ctx) {
    graphers.clear();
    for (int i = 0; i < ctx.GetCount(); i++) {
        MathParser::Equation* eq = ctx.FindEquationIndex(i);
        if (eq && eq->Valid() && eq->EParamCount() == 0)
        {
            //Dont draw constant functions for the time being
            if (eq->IParamCount() == 3)
            {
                //Draw implicit function
                //To do: even implicit and constant functions should get a grapher
            }
            else if (eq->IParamCount() > 0) // 1 or 2
            {
                //Draw explicit function
                int index = graphers.size();
                graphers.emplace_back();
                graphers[index].SetEquation(eq);
            }
            
        }
    }

    // Precalculate the mesh
    for (Grapher3D& g : graphers) {
        g.Calculate(nullptr); //Dont do this every frame
    }
}

int main(int argc, const char* argv[]) {
    LOG_INIT();

    #if CATCH_SIGINT
        signal(SIGINT,  signal_handler);
    #endif
    
    // return 0;
    if (argc == 2) {
        g_strEqFile = argv[1];
    }
    else {
        g_strEqFile = "equations.txt";
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "Graph It", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        LogError ("Failed to initialize OpenGL loader!\n");
        return 1;
    }

    SetCallbacks(window);
    
    //Enable blending and depth buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glDisable(GL_BLEND);

    {
        LogInfo("Window: %d x %d", windowSize.x, windowSize.y);
        LogTrace("Version: %s", glGetString (GL_VERSION));
        LogTrace("Vender: %s", glGetString (GL_VENDOR));
        LogTrace("Renderer: %s", glGetString (GL_RENDERER));
        LogTrace("GLSL: %s", glGetString (GL_SHADING_LANGUAGE_VERSION));
        int nTotalTexSlots;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nTotalTexSlots);
        LogTrace("Texture slots: %d", nTotalTexSlots);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                   ////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    double lastTime = 0;
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    viewport(0, 0, w, h);

    glm::vec3 posCam = glm::vec3(-9.81f, -21.986, 16.197);
    glm::vec3 posTarget = posCam + glm::vec3(0.3228f, 0.738f, -0.5917f);
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
    float fov = 45.0f;
    Camera cam(posCam, posTarget-posCam, up, fov, (float)(windowSize.x) / windowSize.y, 0.1f, 100.0f);
    g_cam = &cam;
    cam.LoadPrefs("Assets/Prefs/cam.pref");

    Renderer* r = new RendererBatch;
    g_renderer = r;

    r->Init(&cam);
    r->PushDepthState(RE_DEPTH_LESS);

    MathParser::Context ctx;
    g_ctx = &ctx;

    ctx.LoadFromFile(g_strEqFile);
    ctx.PrintProperties();

    std::vector<Grapher3D> graphers;

    while (bRunning && !glfwWindowShouldClose(window))
    {
        double curTime = glfwGetTime();
        deltaTime = curTime - lastTime;
        lastTime = curTime;
     
        glfwPollEvents();

        float col = 0.1;
        glClearColor(col, col, col, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cam.ProcessInput(window);
        if (false)
        {
            static float fElapsed = 0.0f;
            float repeat = 1.25;
            fElapsed += deltaTime;
            if (fElapsed > repeat) {
                fElapsed -= repeat;
                double cursorX, cursorY;
                glfwGetCursorPos(window, &cursorX, &cursorY);

                glm::vec3 pos;
                pos.x = Remap(cursorX, 0, windowSize.x, -1.0f, 1.0f);
                pos.y = Remap(cursorY, 0, windowSize.y, 1.0f, -1.0f);
                pos.z = 3.85 / 4.04;
                
                // cam.ScreenPointToRay(glm::vec2(cursorX, cursorY));
                // LogInfo("");
                // Print(pos);
                // glm::vec3 worldpos = cam.NDCToWorldPoint(pos);
                // glm::vec3 worldpos = cam.ScreenToWorldPoint( glm::vec2(cursorX, cursorY) );

                // LogInfo("%+.02f, %+.02f, %+.02f", worldpos.x, worldpos.y, worldpos.z);

                // LogWarn("");
            }

            
        }

        r->StartFrame();

        DrawGrid(r);
        //Lines
        if (false)
        {
            for (float f = -0.9; f <= +0.90001; f += 0.1) {
                r->DrawLine(glm::vec3(-0.9, f, 0.0), glm::vec3(+0.9, f, 0.0), glm::vec4(1.0, 0.0, 0.0, 1.0), 4.0f);

            }
            for (float f = -0.9; f <= +0.90001; f += 0.1) {
                r->DrawLine(glm::vec3(f, -0.9, 0.0), glm::vec3(f, +0.9, 0.0), glm::vec4(0.0, 1.0, 0.0, 1.0), 4.0f);
            }
        }       

        //Dots
        if (false)
        {
            for (float f = -0.9; f <= +0.90001; f += 0.1) {
                for (float g = -0.9; g <= +0.90001; g += 0.1) {
                    glm::vec4 col = glm::vec4(0.0, 0.0, 0.0, 1.0);
                    col.x = Remap(f, -0.9, 0.9, 0.1, 1.0);
                    col.y = Remap(g, -0.9, 0.9, 0.1, 1.0);
                    r->DrawPoint(glm::vec3(f, g, 0.0), col, 2);
                }
            }
        }

        //Dots count
        if (false)
        {
            int max = 10;
            int counter = 0;

            for (float f = +0.9; f >= -0.89999; f -= 0.1) {
                for (float g = +0.9; g >= -0.89999; g -= 0.1) {
                    glm::vec4 col = glm::vec4(0.0, 0.0, 0.0, 1.0);
                    col.x = Remap(f, -0.9, 0.9, 0.1, 1.0);
                    col.y = Remap(g, -0.9, 0.9, 0.1, 1.0);
                    r->DrawPoint(glm::vec3(f, g, 0.0), col, 2);
                    ++counter;

                    if (counter >= max)
                        break;
                }

                if (counter >= max)
                    break;
            }
        }

        //Line loop
        if (false)
        {
            glm::vec3 pos[6];
            pos[0] = glm::vec3(-0.5, -0.7, 0.0);
            pos[1] = glm::vec3(-0.8, +0.0, 0.0);
            pos[2] = glm::vec3(-0.5, +0.7, 0.0);
            pos[3] = glm::vec3(+0.5, +0.7, 0.0);
            pos[4] = glm::vec3(+0.8, +0.0, 0.0);
            pos[5] = glm::vec3(+0.5, -0.7, 0.0);
            
            // r->DrawLineLoop(pos, 1, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2);
            // r->DrawLineLoop(pos, 2, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2);
            // r->DrawLineLoop(pos, 3, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2);
            r->DrawLineLoop(pos, 4, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2);
            r->DrawLineLoop(pos, 5, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2);
            r->DrawLineLoop(pos, 6, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 2);
        }

        //Line strip
        if (false)
        {
            glm::vec3 pos[30];
            int k = 0;
            pos[k++] = glm::vec3(-0.5, -0.7, 0.0);
            pos[k++] = glm::vec3(-0.8, +0.0, 0.0);
            pos[k++] = glm::vec3(-0.5, +0.7, 0.0);
            pos[k++] = glm::vec3(+0.5, +0.7, 0.0);
            pos[k++] = glm::vec3(+0.8, +0.0, 0.0);
            pos[k++] = glm::vec3(+0.5, -0.7, 0.0);
            pos[k++] = glm::vec3(+0.5, -0.7, 0.0);
            pos[k++] = glm::vec3(+0.0, +0.0, 0.0);
            pos[k++] = glm::vec3(+0.2, +0.0, 0.0);
            pos[k++] = glm::vec3(+0.2, +0.2, 0.0);
            pos[k++] = glm::vec3(-0.2, +0.2, 0.0);
            pos[k++] = glm::vec3(-0.2, -0.2, 0.0);
            
            // r->DrawLineStrip(pos, 1, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2);
            // r->DrawLineStrip(pos, 2, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2);
            // r->DrawLineStrip(pos, 3, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2);
            // r->DrawLineStrip(pos, 4, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2);
            
            r->DrawLineStrip(pos, 9, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2);
            r->DrawLineStrip(pos, k, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 2);
        }

        //Triangles
        if (false)
        {
            float z1 = 0.0;
            float z2 = -0.2;

            //r->DrawTriangle(glm::vec3(1, 1, z1), glm::vec3(2, 1, z1), glm::vec3(1, 2, z1), glm::vec4(1.0, 0.2, 0.2, 1.0));
            r->DrawTriangle(glm::vec3(2, 0, 0), glm::vec3(0, 2, 0), glm::vec3(1, 0, 1), glm::vec4(0.2, 1.0, 0.2, 1.0));
        }

        //Triangle Strip
        if (false)
        {
            glm::vec3 pos[30];
            int k = 0;
            //Square
            pos[k++] = glm::vec3(1, 1, 0.0);
            pos[k++] = glm::vec3(2, 2, 0.0);
            pos[k++] = glm::vec3(3, 1, 0.0);
            pos[k++] = glm::vec3(3, 1, 1);
            pos[k++] = glm::vec3(4, 1, 1);
            pos[k++] = glm::vec3(4, 2, 1);
             
            //int count = 25;
            //for (int i = 0; i < count; i++) {
            //    float offx = 2.0 * ((float)(i+1) / count);
            //    float offy = (i%2 == 1) ? +0.5 : -0.5;
            //    pos[k++] = glm::vec3(1 + offx, 1 + offy, 0.0);
            //}
            
            //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            r->DrawTriangleStrip(pos, k, glm::vec4(0.2, 0.8, 0.1, 1.0));

            //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }

        //Triangle fan
        if (false)
        {
            glm::vec3 pos[100];
            int k = 0;
            //Square
            // pos[k++] = glm::vec3(-0.5, -0.5, 0.0);
            // pos[k++] = glm::vec3(+0.5, -0.5, 0.0);
            // pos[k++] = glm::vec3(-0.5, +0.5, 0.0);
            // pos[k++] = glm::vec3(+0.5, +0.5, 0.0);

            float degrees = 180;
            float increment = degrees / 20.0f;
            float rad = 0.5;
            pos[k++] = glm::vec3(+0.0, +0.0, 0.0);
            for (float f = 0.0f; f <= degrees + 0.1f; f += increment) {
                float radians = glm::radians(f);
                float x = glm::cos(radians) * rad;
                float y = glm::sin(radians) * rad;
                pos[k++] = glm::vec3(x, y, 0.0);
            }
            
            // r->DrawLineStrip(pos, k, glm::vec4(1.0, 0.1, 0.1, 1.0), 1.0f);

            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            r->DrawTriangleFan(pos, k, glm::vec4(1.0, 0.1, 0.1, 1.0));
            // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }

        //Quad
        if (false) {
            glm::vec3 pos[] = {
                glm::vec3(+1.0, +1.0, +0.0),
                glm::vec3(+2.0, +1.0, +0.0),
                glm::vec3(+1.0, +2.0, +0.0),
                glm::vec3(+2.0, +2.0, +0.0),
            };

            // LogWarn("");
            // for (int i = 0; i < 4; i++) {
            //     glm::vec4 res = cam.VP() * glm::vec4(pos[i], 1.0f);
            //     res /= res.w;
            //     LogInfo("Res: %+.02f, %+.02f, %+.02f", res.x, res.y, res.z);
            // }

            // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            r->DrawTriangleStrip(pos, 4, glm::vec4(0.2, 0.8, 0.2, 1.0));
        }

        if (g_updateGrapher)
        {
            g_updateGrapher = false;
            UpdateGraphers(graphers, ctx);
        }

        for (Grapher3D& g : graphers) {
            // g.Calculate(r); //Dont do this every frame
            g.Draw(r);
        }

        r->EndFrame();

        glfwSwapBuffers(window);
    }

    r->PopDepthState();
    // Cleanup
    delete r;
    r = nullptr;
    g_renderer = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();
}