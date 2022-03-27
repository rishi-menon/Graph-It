#pragma once
#include "DebugFinal.h"
#include "MathNode.h"

#include <string>
#include <vector>
#include <array>
#include <variant>
#include <stack>

namespace MathParser {

enum TokenType {
    Token_Invalid,
    Token_Number,
    Token_EqualSign,
    
    Token_Expression,

    Token_Operator, //Mainly used to refer to binary operators
    Token_Comma,

    Token_Param_Start,
    Token_Param_End,

    Token_Brack_Start,
    Token_Brack_End,
};

struct TokenData {
    TokenType Type;

    std::string_view Str;
    double NumberValue;
    char charValue;
};

class Context;

class Equation {
public:
    Equation() = default;
    Equation(Context* ctx):
        myContext(ctx)
    {
    }
    
    void PushNode(const NodeGeneric& n) {
        Assert(myNodeCount < myNodeSize && "Ran out of nodes");
        myNodes[myNodeCount] = n;
        myNodeCount++;
    }

    void SetName(std::string_view name) { myEquationName = name; }
    const std::string_view& Name() { return myEquationName; }

    void Print() const;
    void ResolveEquations(Context* ctx);    // Fetches equations from the ctx
    
    // Calculates IParamCount, EParamCount and validity
    void FetchProperties();
    int EParamCount() const { return myEParamCount; }
    int IParamCount() const { return myIParamCount; }
    bool Valid() const { return myIsValid; }


    double Evaluate(double x, double y);
    double Evaluate(double x, double y, double z);

    //Todo: Make this private and accessible from MathExpression
    NodeValueType EvaluatePrivate(NodeValue* iParams, int iSize, NodeValue* eParams, int eSize);
private:

private:
    Context* myContext = nullptr;
    std::string_view myEquationName;

    static constexpr int myNodeSize = 100;
    NodeGeneric myNodes[myNodeSize];
    int myNodeCount = 0;

    //Properties
    int myEParamCount = 0; //Number of explicit parameters that this equation has
    int myIParamCount = 0; //Number of implicit parameters that this equation has
    bool myIsValid = false;
};

class Context {
public:
    Context();
    ~Context();
    Context(const Context&) = delete;
    Context& operator= (const Context&) = delete;
    
    //Moving is allowed
    Context& operator= (Context&& other);

    void Clear();
    void PrintProperties(bool bPrintBuiltIn = false);

    bool AddEquation(const std::string& str);
    bool LoadFromFile(const std::string& str);

    bool Resolve();
    Equation* FindEquation(const std::string_view& str);
    Equation* FindEquationIndex(int index);

    static bool RunAllTests();    //Returns true when all tests pass

    int GetCount() const { return myCount; }

private:
    bool ParseInfixToTokens(const std::string& strEquation, std::vector<TokenData>& outInfix, std::string* outError);
    bool InfixToPostFix(std::vector<TokenData>& infix, std::string* outError, Equation* outEq);

    bool AddEquationPrivate(const std::string& strEquation, std::string* outError, Equation* eq);

    //Debug related
    void PrintTokens(const std::vector<TokenData>& tokens);
    bool RunTest_InfixToToken();

    void ClearPrivate();
    void AddInbuiltEqs();

//Variables
private:
    // TokenData and Nodes store string_views. We need a storage for the string that the string_view points to.
    // We cannot make it a std::vector because those might get resized and the string_view might become invalid 
    static constexpr int myMaxEquations = 40;
    int myCount = 0;
    int myCustomEqStart = 0;    //Index of the first non inbuilt equation. This is only used for printing properties of equations
    Equation* myEquations[myMaxEquations];


//Todo: Make this private
public:
    std::string myStrEquations[myMaxEquations];
    
};

}