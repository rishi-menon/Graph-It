#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void SetCallbacks(GLFWwindow* window);

void WindowResizeCallback(GLFWwindow* pWindow, int nWidth, int nHeight);
void WindowPosCallback(GLFWwindow* window, int x, int y);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

