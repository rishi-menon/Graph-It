#pragma once
#include "DebugFinal.h"
#include <string>
#include <vector>
#include <array>
#include <variant>
#include <stack>

namespace MathParser {

enum class NodeType {
    Node,
    NodeValue,
    NodeParam,
    NodeOperator,
    NodeExpression
};


struct NodeGeneric;
class Equation;
class Context;

class NodeValue {
public:
    NodeValue() = default;
    NodeValue(double d):
        value(d)
    {
    }

    void Print() const {
        Log("%-15s : %+.04f\n", "Number", value);
    }

    void SetValue(double d) { value = d; }
    double GetValue() const { return value; }

private:
    double value;
};

struct NodeValueType {
    bool Success;
    NodeValue Value;

    NodeValueType(bool success, double val=0.0):
        Success(success), Value(val)
    {}

    NodeValueType(double val):
        Success(true), Value(val)
    {}

};

class NodeParam {
public:
    NodeParam() = default;
    NodeParam(int index, bool implicit):
        myIndex(index), myIsImplicit(implicit)
    {
    }

    void Print() const {
        Log("%-15s : %d (%s)\n", "Param", myIndex, (myIsImplicit ? "Implicit" : "Explicit") );
    }

    int Index() const { return myIndex; }
    bool Implicit() const { return myIsImplicit; }

private:
    int myIndex;
    bool myIsImplicit;
};

class NodeOperator {
public:
    enum Operator{
        OP_INVALID,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_POW,

        //Custom
        OP_SIN,
        OP_COS,
        OP_TAN,

        OP_SQRT,
        OP_EXP,
    };

public:
    NodeOperator() = default;
    NodeOperator(char op) :
        myOp( CharToOP(op) )
    {
    }
    NodeOperator(Operator op) :
        myOp(op)
    {
        
    }

    void Print() const {
        Log("%-15s : %c\n", "Operator", myOp);
    }

    NodeValueType Calculate(std::stack<NodeValue>& values);

private:
    Operator CharToOP(char c);

private:
    Operator myOp;
};

//This can either be a const variable or a float
class NodeExpression {
public:
    NodeExpression() = default;
    NodeExpression(const std::string_view& str):
        myEquation(nullptr), myName(str)
    {
    }

    NodeExpression(const std::string_view& str, std::vector<Equation>&& eqs):
        myEquation(nullptr), myName(str), myParams( std::move(eqs) )
    {   
    }

    // void SetEquation(Equation* eq) { myEquation = eq; }
    void Print() const;
    
    const std::string_view& Name() const { return myName; }
    Equation* GetEquation() { return myEquation; }
    std::vector<Equation>& GetParams() { return myParams; }
    const Equation* GetEquation() const { return myEquation; }
    const std::vector<Equation>& GetParams() const { return myParams; }

    void ResolveEquations(Context* ctx);
    void FetchProperties();
    
    NodeValueType Calculate(NodeValue* iParams, int iSize, NodeValue* eParams, int eSize);

private:
    Equation* myEquation;
    std::string_view myName;
    std::vector<Equation> myParams;

};

struct NodeGeneric {
    NodeType type;
    std::variant<NodeValue, NodeParam, NodeOperator, NodeExpression> data;

    NodeGeneric() = default;
    
    NodeGeneric(NodeValue n):
        type(NodeType::NodeValue), data(n)
    {}
    NodeGeneric(NodeParam n):
        type(NodeType::NodeParam), data(n)
    {}
    NodeGeneric(NodeOperator n):
        type(NodeType::NodeOperator), data(n)
    {}

    NodeGeneric(const NodeExpression& n):
        type(NodeType::NodeExpression), data(n)
    {}

    

    NodeGeneric& operator= (NodeValue n) {
        type = NodeType::NodeValue;
        data = n;
    }

    NodeGeneric& operator= (NodeParam n) {
        type = NodeType::NodeParam;
        data = n;
    }
    NodeGeneric& operator= (NodeOperator n) {
        type = NodeType::NodeOperator;
        data = n;
    }
    NodeGeneric& operator= (const NodeExpression& n) {
        type = NodeType::NodeExpression;
        data = n;
    }




    void Print() const;
    const NodeValue* GetValue() const { return std::get_if<NodeValue>(&data); }
    const NodeParam* GetParam() const { return std::get_if<NodeParam>(&data); }
    const NodeOperator* GetOp() const { return std::get_if<NodeOperator>(&data); }
    const NodeExpression* GetExpr() const { return std::get_if<NodeExpression>(&data); }
    NodeValue* GetValue() { return std::get_if<NodeValue>(&data); }
    NodeParam* GetParam() { return std::get_if<NodeParam>(&data); }
    NodeOperator* GetOp() { return std::get_if<NodeOperator>(&data); }
    NodeExpression* GetExpr() { return std::get_if<NodeExpression>(&data); }
};


} // End of namespace
