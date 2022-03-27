#include "MathNode.h"
#include "MathContext.h"

#include <unordered_map>
#include <stack>
#include <cmath>
#include "Maths.h"

using std::vector;
using std::string_view;

namespace MathParser {

NodeOperator::Operator NodeOperator::CharToOP(char c) {
    switch (c) {
        case '+': return OP_ADD;
        case '-': return OP_SUB;
        case '*': return OP_MUL;
        case '/': return OP_DIV;
        case '^': return OP_POW;
        
        default:  return OP_INVALID;
    }
}

NodeValueType NodeOperator::Calculate(std::stack<NodeValue>& values) {
    switch (myOp)
    {
        case OP_ADD: {
            if (values.size() < 2)
                return NodeValueType(false);

            double op2 = values.top().GetValue();
            values.pop();
            double op1 = values.top().GetValue();
            values.pop();

            return NodeValueType( op1 + op2 );
        }
        case OP_SUB: {
            if (values.size() < 2)
                return NodeValueType(false);
            
            double op2 = values.top().GetValue();
            values.pop();
            double op1 = values.top().GetValue();
            values.pop();

            return NodeValueType( op1 - op2 );
        }
        case OP_MUL: {
            if (values.size() < 2)
                return NodeValueType(false);
            
            double op2 = values.top().GetValue();
            values.pop();
            double op1 = values.top().GetValue();
            values.pop();

            return NodeValueType( op1 * op2 );
        }
        case OP_DIV: {
            if (values.size() < 2)
                return NodeValueType(false);
            
            double op2 = values.top().GetValue();
            values.pop();
            double op1 = values.top().GetValue();
            values.pop();

            return NodeValueType( op1 / op2 );
        }
        case OP_POW: {
            if (values.size() < 2)
                return NodeValueType(false);
            
            double op2 = values.top().GetValue();
            values.pop();
            double op1 = values.top().GetValue();
            values.pop();

            return NodeValueType( std::pow(op1, op2) );
        }

        case OP_SIN: {
            if (values.size() < 1)
                return NodeValueType(false);

            double op = values.top().GetValue();
            values.pop();
            return NodeValueType( glm::sin(op) );
        }
        case OP_COS: {
            if (values.size() < 1)
                return NodeValueType(false);

            double op = values.top().GetValue();
            values.pop();
            return NodeValueType( glm::cos(op) );
        }
        case OP_TAN: {
            if (values.size() < 1)
                return NodeValueType(false);

            double op = values.top().GetValue();
            values.pop();
            return NodeValueType( glm::tan(op) );
        }

        case OP_SQRT: {
            if (values.size() < 1)
                return NodeValueType(false);

            double op = values.top().GetValue();
            values.pop();
            return NodeValueType( glm::sqrt(op) );
        }

        case OP_EXP: {
            if (values.size() < 1)
                return NodeValueType(false);

            double op = values.top().GetValue();
            values.pop();
            return NodeValueType( glm::exp(op) );
        }
    }

    LogError("Unknown type: %c (%d)", myOp, myOp);
    Assert(false);
    return NodeValueType(false);
}

void NodeExpression::ResolveEquations(Context* ctx) {
    Equation* pEq = ctx->FindEquation ( myName );
    myEquation = pEq;
    if (!pEq)
        return;

    for (Equation& eq : myParams) {
        eq.ResolveEquations(ctx);
    }
}

void NodeExpression::FetchProperties() {
    Assert(myEquation);
    myEquation->FetchProperties();
    for (Equation& eq : myParams) {
        eq.FetchProperties();
    }
}

NodeValueType NodeExpression::Calculate(NodeValue* iParams, int iSize, NodeValue* eParams, int eSize)
{
    if (!myEquation)
        return NodeValueType(false);
    
    //Todo: Store this on the stack
    std::vector<NodeValue> funcParams;
    funcParams.reserve(20);

    for (Equation& eq : myParams) {
        NodeValueType res = eq.EvaluatePrivate(iParams, iSize, eParams, eSize);
        if (!res.Success)
            return res;
        funcParams.push_back(res.Value);
    }
    return myEquation->EvaluatePrivate(iParams, iSize, funcParams.data(), funcParams.size());
}


void NodeGeneric::Print() const {
    if (GetValue())
        GetValue()->Print();

    if (GetParam())
        GetParam()->Print();

    if (GetOp())
        GetOp()->Print();

    if (GetExpr())
        GetExpr()->Print();
}

void NodeExpression::Print() const {
    Log("%-15s : ", "Expr");
    for (char c : myName) {
        Log("%c", c);
    }
    if (myParams.size()) {
        Log("()");
    }
    Log("\n");
    
    if (myParams.size()) {
        Log("\n");
        for (const Equation& eq : myParams) {
            eq.Print();
        }
        Log("\n");
    }
}

} //End of namespace MathParser