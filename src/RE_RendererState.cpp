#include "RE_RendererState.h"
#include <GL/glew.h>

bool glCheckError_(int line, const char* file) {
    GLenum errorCode;
    int nTries = 0;
    while ( (errorCode = glGetError()) != GL_NO_ERROR )
    {
        if (++nTries > 50) {
            LogWarn("Infinite loop detected");
            break;
        }

        const char* strError;
        switch (errorCode) {
        case GL_INVALID_ENUM:                  strError = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 strError = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             strError = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                strError = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               strError = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 strError = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: strError = "INVALID_FRAMEBUFFER_OPERATION"; break;

        default: strError = "Unknown error"; break;
        }
        LogL(LOG_LEVEL_ERROR, LOG_COL_ERROR "%s:%d ~ OpenGL Error(0x%X): %s" LOG_COL_TRACE LOG_ENDL, file, line, errorCode, strError);
        ASSERT_IDE();
    }

    return (nTries != 0);
}


void RendererState::Init() {
    myFlush = false;
    glEnable(GL_DEPTH_TEST);
    PushDepthState(RE_DEPTH_ALWAYS);

    PushPolygonState(RE_POLYGON_FILL);
}

//-----------------------------------------------------
//               Depth
//-----------------------------------------------------

void RendererState::SetDepthStatePrivate(DepthState s) {
    GLenum val;
    switch (s) {
    default: Assert(false && "Unknown type"); return;
    case RE_DEPTH_NEVER: val = GL_NEVER;     break;
    case RE_DEPTH_LESS: val = GL_LESS;      break;
    case RE_DEPTH_EQUAL: val = GL_EQUAL;     break;
    case RE_DEPTH_LEQUAL: val = GL_LEQUAL;    break;
    case RE_DEPTH_GREATER: val = GL_GREATER;   break;
    case RE_DEPTH_NOTEQUAL: val = GL_NOTEQUAL;  break;
    case RE_DEPTH_GEQUAL: val = GL_GEQUAL;    break;
    case RE_DEPTH_ALWAYS: val = GL_ALWAYS;    break;
    }
    glDepthFunc(val);
    myDepthStateStack.SetTop(s);
    glCheckError();
}

void RendererState::SetDepthState(DepthState s) {
    if (s != myDepthStateStack.Top() && s != RE_DEPTH_INVALID) {
        Flush();
        SetDepthStatePrivate(s);
    }
}

void RendererState::PopDepthState() {
    DepthState prev = myDepthStateStack.Pop();
    if (!myDepthStateStack.Empty()) {
        DepthState cur = myDepthStateStack.Top();
        if (prev != cur && cur != RE_DEPTH_INVALID) {
            Flush();
            SetDepthStatePrivate(cur);
        }
    }
}


//-----------------------------------------------------
//               Polygon State
//-----------------------------------------------------

void RendererState::SetPolygonStatePrivate(PolygonState s) {
    GLenum val;
    switch (s) {
        default                  : Assert(false && "Unknown type"); return;
        case RE_POLYGON_POINT    : val = GL_POINT;     break;
        case RE_POLYGON_LINE     : val = GL_LINE;      break;
        case RE_POLYGON_FILL     : val = GL_FILL;      break;
    };
    glPolygonMode( GL_FRONT_AND_BACK, val );
    myPolygonStateStack.SetTop(s);
    glCheckError();
}

void RendererState::SetPolygonState(PolygonState s) {
    if (s != RE_POLYGON_INVALID && s != myPolygonStateStack.Top()) {
        Flush();
        SetPolygonStatePrivate(s);
    }
}

void RendererState::PopPolygonState() {
    PolygonState prev = myPolygonStateStack.Pop();
    if (!myDepthStateStack.Empty()) {
        PolygonState cur = myPolygonStateStack.Top();
        if (prev != cur && cur != RE_DEPTH_INVALID) {
            Flush();
            SetPolygonStatePrivate(cur);
        }
    }
}
