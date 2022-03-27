#include "RE_Renderer.h"
#include <type_traits>

//Renderer::Renderer() :
//{
//
//}
Renderer::~Renderer() {
    Cleanup();
}

void Renderer::Init(Camera* cam) {
    RendererState::Init();
    myCam = cam;
}
void Renderer::Cleanup() {

}

