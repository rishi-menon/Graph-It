#include "EventCallback.h"
#include "DebugFinal.h"
#include "Camera.h"
#include "RE_Renderer.h"
#include "MathContext.h"

extern void ResetViewport();
extern void viewport(int x, int y, int w, int h);

extern Camera* g_cam;
extern Renderer* g_renderer;
extern const char* g_strEqFile;
extern MathParser::Context* g_ctx;
extern bool g_updateGrapher;

void SetCallbacks(GLFWwindow* window) {
    // glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, WindowResizeCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowPosCallback(window, WindowPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, MousePositionCallback);
	glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);
}

void WindowResizeCallback(GLFWwindow* pWindow, int width, int height)
{
	// LogTrace("Window Resize Event: width: %d height: %d", width, height);
    // ResetViewport();
    
	#ifdef RM_MAC
		//mac has this weird issue where the renderer draws only on part of the screen when resizing
		// int x, y;
		// glfwGetWindowPos (pWindow, &x, &y);
		// glfwSetWindowPos(pWindow, x+1, y);
		// glfwSetWindowPos(pWindow, x, y);
	#endif
}

void WindowPosCallback(GLFWwindow* window, int x, int y)
{
	// LogInfo("Window Move Event: %d, %d", x, y);

    ResetViewport();
}


void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// LogInfo("Framebuffer Resize Event: %d x %d", width, height);
    // ResetViewport();
    viewport(0, 0, width, height);
}


void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
     //LogTrace("Mouse Press Event: %d, %d, %d", button, action, mods);

	switch (action)
	{
		case GLFW_PRESS:
		{
			//Mouse down
			break;
		}
		case GLFW_RELEASE:
		{
			//Mouse up
			break;
		}
	}
}

void MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	// LogTrace("Mouse Move Event: %f, %f", xpos, ypos);
}

void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	// LogTrace("Mouse Scroll Event: %f, %f", xoffset, yoffset);
    if (g_cam)
        g_cam->ProcessZoom(yoffset);

}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// LogTrace("Key Press Event: %d, %d, %d, %d", key, scancode, action, mods);

	switch (action)
	{
		case GLFW_PRESS:
		{
			//Key Down
			switch (key) {
                case GLFW_KEY_C: {
                    if (g_cam)
				    	g_cam->SavePrefs("Prefs/cam.pref");
                    break;
                }

                case GLFW_KEY_M: {
                    g_renderer->PrintMetrics();
                    break;
                }

                case GLFW_KEY_R: {
                    g_ctx->Clear();
                    g_ctx->LoadFromFile(g_strEqFile);
                    g_ctx->PrintProperties();
                    g_updateGrapher = true;
                    break;
                }
                
                case GLFW_KEY_P: {
                    g_ctx->PrintProperties(true);
                    break;
                }
			}
			break;
		}
		case GLFW_RELEASE:
		{
			//Key Up
			break;
		}
		case GLFW_REPEAT:
		{
			//Key pressed
			break;
		}
	}
}