#include "Camera.h"
#include "Maths.h"
#include "RE_Renderer.h"
#include <fstream>
#include <filesystem>

Camera::Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, float fov, float aspectWidthByHeight, float zNear, float zFar) :
    myValid(false),
    myPos(pos),
    myDir( glm::normalize(dir) ),
    myUp( glm::normalize(up) ),
    myFov(fov),
    myAspect(aspectWidthByHeight),
    myZNear(zNear),
    myZFar(zFar),

    myOldMouseX(0),
    myOldMouseY(0)

{
    Validate();
}

void Camera::SavePrefs(const char* file) {
    //namespace fs = std::filesystem;
    //fs::create_directories(file);
    FILE* pf = fopen(file, "w");
    if (pf)
    {
        fwrite((const void*)(&myPos.x), sizeof(glm::vec3), 1, pf);
        fwrite((const void*)(&myDir.x), sizeof(glm::vec3), 1, pf);
        fwrite((const void*)(&myUp.x), sizeof(glm::vec3), 1, pf);
        fwrite((const void*)(&myFov), sizeof(float), 1, pf);
        fwrite((const void*)(&myAspect), sizeof(float), 1, pf);
        fwrite((const void*)(&myZNear), sizeof(float), 1, pf);
        fwrite((const void*)(&myZFar), sizeof(float), 1, pf);

        LogInfo("Saved camera prefs to: %s", file);
        fclose(pf);
    }
    else
    {
        LogInfo("Could not save camera prefs to: %s", file);
    }
}
void Camera::LoadPrefs(const char* file) {
    FILE* pf = fopen(file, "r");
    if (pf)
    {
        constexpr int totalSize = 3 * sizeof(glm::vec3) + 4 * sizeof(float);
        uint8 buf[totalSize] = {};

        if (fread(buf, sizeof(uint8), totalSize, pf) == totalSize) {
            uint8* p = buf;
            memcpy(&myPos.x, p, sizeof(glm::vec3)); p += sizeof(glm::vec3);
            memcpy(&myDir.x, p, sizeof(glm::vec3)); p += sizeof(glm::vec3);
            memcpy(&myUp.x, p, sizeof(glm::vec3)); p += sizeof(glm::vec3);
            memcpy(&myFov, p, sizeof(float));      p += sizeof(float);
            memcpy(&myAspect, p, sizeof(float));      p += sizeof(float);
            memcpy(&myZNear, p, sizeof(float));      p += sizeof(float);
            memcpy(&myZFar, p, sizeof(float));      p += sizeof(float);
            
            LogInfo("Read camera prefs from: %s", file);
        }
        else {
            LogWarn("Could not read enough bytes from pref file: %s", file);
        }

        fclose(pf);
    }
}


void Camera::CalculateMatrix() {
    myMatView = glm::lookAt( myPos, myPos+myDir, myUp);
    myMatProj = glm::perspective(glm::radians(myFov), myAspect, myZNear, myZFar);

    myMatViewInv = glm::inverse(myMatView);
    myMatProjInv = glm::inverse(myMatProj);
    myMatVP = myMatProj * myMatView;
    myMatVPInv = myMatViewInv * myMatProjInv;
}

glm::vec3 Camera::NDCToWorldPoint(glm::vec3 position) const {
    Assert(myValid);
    glm::vec4 pos = glm::vec4(position, 1.0f);
    pos = myMatVPInv * pos;
    pos /= pos.w;
    return glm::vec3(pos);
}
glm::vec3 Camera::WorldToNDCPoint(glm::vec3 position) const {
    Assert(myValid);
    glm::vec4 pos = glm::vec4(position, 1.0f);
    pos = myMatVP * pos;
    pos /= pos.w;
    return glm::vec3(pos);
}
glm::vec3 Camera::ScreenToWorldPoint(glm::vec2 position) const {
    glm::vec3 pos;
    pos.x = Remap(position.x, 0, windowSize.x, -1.0f, 1.0f);
    pos.y = Remap(position.y, 0, windowSize.y, 1.0f, -1.0f);
    //To do: Get z from the depth buffer
    // pos.z = 

    Assert(false && "Todo: Implement function");
    return NDCToWorldPoint(pos);
    // GLint val = 0;
    // glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER , GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &val);
    // LogInfo("Bits: %d", val);

    // uint data;
    // glReadPixels((GLint)(position.x), (GLint)(position.y), 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &data);
    // LogInfo("Data: %d", data);
    // LogInfo("Data: %+.03f", data);
    // glCheckError();
}

glm::vec3 Camera::ScreenPointToRay(glm::vec2 position) const {
    glm::vec3 pos;
    pos.x = Remap(position.x, 0, windowSize.x, -1.0f, 1.0f);
    pos.y = Remap(position.y, 0, windowSize.y, 1.0f, -1.0f);
    pos.z = -1.0f;  //Near plane
    glm::vec3 posNear = NDCToWorldPoint(pos);
    pos.z = +1.0f;  //far plane
    glm::vec3 posFar = NDCToWorldPoint(pos);

    glm::vec3 dir = posFar - posNear;
    glm::vec3 dirCam = posNear - myPos;
    glm::vec3 dirCam2 = posFar - myPos;

    LogWarn("Pos N: %+.02f, %+.02f, %+.02f", posNear.x, posNear.y, posNear.z);
    LogWarn("Pos F: %+.02f, %+.02f, %+.02f", posFar.x, posFar.y, posFar.z);
    LogWarn("Pos  : %+.02f, %+.02f, %+.02f", myPos.x, myPos.y, myPos.z);

    LogWarn("Dir 0: %+.02f, %+.02f, %+.02f", dir.x, dir.y, dir.z);
    LogWarn("Dir 1: %+.02f, %+.02f, %+.02f", dirCam.x, dirCam.y, dirCam.z);
    LogWarn("Dir 2: %+.02f, %+.02f, %+.02f", dirCam2.x, dirCam2.y, dirCam2.z);

    Assert(false && "Todo: implement this");

    return glm::normalize(posFar - posNear);
}

void Camera::ProcessZoom(double yoff)
{
    if (yoff == 0.0)
        return;

    double scale = 1.0f;
    myPos += myDir * float(yoff / scale);
    Validate(true);
}

void Camera::ProcessInput(GLFWwindow* win) {
    double x,y;
    glfwGetCursorPos(win, &x, &y);

    double deltaX = x - myOldMouseX; 
    double deltaY = myOldMouseY - y; 
    myOldMouseX = x;
    myOldMouseY = y;

    //Left mouse button
    if (glfwGetMouseButton(win, 0))
    {
        if (deltaX != 0.0 || deltaY != 0.0)
        {
            const double scaleX = 300;
            const double scaleY = 300;

            // LogInfo("%+.02f, %+.02f", deltaX, deltaY);
            glm::vec3 forward = myDir;
            glm::vec3 right = glm::normalize(glm::cross(forward, myUp));
            glm::vec3 up = glm::cross(right, forward);

            myDir += up * (float)(deltaY / scaleY) + right * (float)(deltaX / scaleX);
            myDir = glm::normalize(myDir);
            
            
            double sqmag = SqMag (glm::cross(myDir, myUp));
            if (sqmag <= 1e-2 && false) {
                //We need to recalculate our myUp vector because the camera is very very close to it
                // Assert(false);
                // LogWarn("Recalculating up");
                forward = myDir;
                right = glm::normalize(glm::cross(forward, myUp));
                up = glm::cross(right, forward);
                
                myUp = up;
            }

            // LogWarn("");
            // LogInfo("%+.02f, %+.02f, %+.02f", myDir.x, myDir.y, myDir.z);
            // LogInfo("%+.02f, %+.02f, %+.02f", myUp.x, myUp.y, myUp.z);
            // LogInfo("%+.02f, %+.02f, %+.02f", vec.x, vec.y, vec.z);
            
            // LogInfo("Magnitude: %+.02f", alength);

        }
    }

    if (glfwGetMouseButton(win, 2))
    {
        if (deltaX != 0.0 || deltaY != 0.0)
        {
            const double scaleX = 300;
            const double scaleY = 300;

            // LogInfo("%+.02f, %+.02f", deltaX, deltaY);
            glm::vec3 forward = myDir;
            glm::vec3 right = glm::normalize(glm::cross(forward, myUp));
            glm::vec3 up = glm::cross(right, forward);

            myPos += up * (float)(-deltaY / scaleY) + right * (float)(-deltaX / scaleX);

        }

    }

    Validate(true);
}
