#pragma once
#include "Maths.h"
#include "RE_Buffers.h"
#include <GLFW/glfw3.h>

class Camera;
extern double deltaTime;
extern glm::ivec2 windowSize;

class Camera {
public:
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, float fov, float aspectWidthByHeight, float zNear, float zFar);

    const glm::mat4& Proj() const                   { Assert(myValid); return myMatProj; }
    const glm::mat4& ProjInv() const                { Assert(myValid); return myMatProjInv; }
    const glm::mat4& View() const                   { Assert(myValid); return myMatView; }
    const glm::mat4& ViewInv() const                { Assert(myValid); return myMatViewInv; }
    const glm::mat4& VP() const                     { Assert(myValid); return myMatVP; }
    const glm::mat4& VPInv() const                  { Assert(myValid); return myMatVPInv; }

    glm::vec3 NDCToWorldPoint(glm::vec3 pos) const;
    glm::vec3 WorldToNDCPoint(glm::vec3 pos) const;
    glm::vec3 ScreenToWorldPoint(glm::vec2 pos) const;
    glm::vec3 ScreenPointToRay(glm::vec2 pos) const;

    void ProcessInput(GLFWwindow* win);
    void ProcessZoom(double yoff);
    
    void LoadPrefs(const char* file);
    void SavePrefs(const char* file);

private:
    void CalculateMatrix();
    inline void Validate(bool bForce = false) {
        if (bForce || !myValid) {
            CalculateMatrix();
            myValid = true;
        }
    }

private:
    //Prefs that need to be saved
    glm::vec3           myPos;
    glm::vec3           myDir;
    glm::vec3           myUp;
    float               myFov;
    float               myAspect;
    float               myZNear;
    float               myZFar;


    bool                myValid;
    //Todo: remove some of the matrices. We don't need to store so many
    glm::mat4           myMatView;
    glm::mat4           myMatProj;
    glm::mat4           myMatViewInv;
    glm::mat4           myMatProjInv;
    glm::mat4           myMatVP;
    glm::mat4           myMatVPInv;

    //Variables to turn the camera
    double myOldMouseX;
    double myOldMouseY;

};