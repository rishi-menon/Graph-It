#pragma once
#include "DebugFinal.h"
#include "Maths.h"
#include <vector>
#include "RE_Renderer.h"
#include "MathContext.h"

//Marching squares
class Grapher3D {
public:
    Grapher3D() = default;
    Grapher3D(const Grapher3D&) = default;

    using FuncExplicitType = double(*)(double x, double y);
    using FuncImplicitType = double(*)(double x, double y, double z);


    //Todo: take a delegate instead of storing an Equation* as a member
    void Calculate(Renderer* r);
    void CalculateExplicit(Renderer* r);

    void CalculateImplicit(FuncImplicitType func);

    void Draw(Renderer* r);

    void SetEquation(MathParser::Equation* eq) { myEquation = eq; }

private:
    struct TriangleStrip {
        std::vector<glm::vec3> Positions;
    };

    std::vector<TriangleStrip> myStrips;

    //Todo: Store a delegate instead of a Equation*
    MathParser::Equation* myEquation;
};