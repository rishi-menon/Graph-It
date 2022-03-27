#pragma once
#include "DebugFinal.h"
#include "Maths.h"

bool glCheckError_(int line, const char* file);
#define glCheckError() glCheckError_(__LINE__, __FILE__)


constexpr int DepthStateSize = 16;
constexpr int PolygonStateSize = 4;

enum DepthState {
    RE_DEPTH_INVALID,
    RE_DEPTH_NEVER,
    RE_DEPTH_LESS,
    RE_DEPTH_EQUAL,
    RE_DEPTH_LEQUAL,
    RE_DEPTH_GREATER,
    RE_DEPTH_NOTEQUAL,
    RE_DEPTH_GEQUAL,
    RE_DEPTH_ALWAYS,
};

enum PolygonState {
    RE_POLYGON_INVALID,
    RE_POLYGON_POINT,
    RE_POLYGON_LINE, //Wireframe mode
    RE_POLYGON_FILL,  //Reguar mode
};




//Type is meant to be an enum or a simple struct
template<typename Type, int64 N>
class StateStack {
public:
    StateStack() :
        myStates{}, myPos(0)
    {
    }
    StateStack(const StateStack&) = default;

    void Push(Type t) {
        Assert(myPos < N && "Stack is full");
        myStates[myPos] = t;
        ++myPos;
    }
    //Pushes the top most element to the stack. If empty then pushes the paramter instead
    void PushTop(Type empty) {
        Push( Empty() ? empty : Top() );
    }

    void SetTop(Type t) {
        Assert(!Empty() && "Stack is empty");
        myStates[myPos-1] = t;
    }

    Type Pop() {
        Assert(!Empty() && "Stack is empty");
        --myPos;
        Type t = myStates[myPos];
        myStates[myPos] = Type();
        return t;
    }

    Type Top() const {
        Assert(!Empty() && "Stack is empty");
        return myStates[myPos-1];
    }

    bool Empty() const {
        Assert(myPos >= 0);
        return (myPos == 0);
    }

private:
    Type myStates[N];
    int64 myPos;
};





class RendererState {
public:
    RendererState() = default;
    virtual ~RendererState() { }
    RendererState(const RendererState&) = delete;
    
    void Init();

    //Depth state
    void PushDepthState(DepthState s = RE_DEPTH_INVALID) 
        { myDepthStateStack.PushTop(RE_DEPTH_INVALID); SetDepthState(s); }
    void SetDepthState(DepthState s);
    void PopDepthState();

    void PushPolygonState(PolygonState s = RE_POLYGON_INVALID)
        { myPolygonStateStack.PushTop(RE_POLYGON_INVALID); SetPolygonState(s); }
    void SetPolygonState(PolygonState s);
    void PopPolygonState();

private:
    void SetDepthStatePrivate(DepthState s);
    void SetPolygonStatePrivate(PolygonState s);

protected:
    virtual void DoFlush() = 0;
    inline void Flush() {
        if (myFlush) {
            myFlush = false;
            DoFlush();
        }
    }

//Member variables
protected:
    bool myFlush;

private:
    StateStack<DepthState, DepthStateSize>               myDepthStateStack;
    StateStack<PolygonState, PolygonStateSize>           myPolygonStateStack;
};