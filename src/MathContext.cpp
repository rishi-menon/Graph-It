
#include "MathContext.h"
#include "MathNode.h"

#include <unordered_map>
#include <stack>
#include <fstream>

#include "Maths.h"

using std::vector;
using std::string_view;
using MapParams = std::unordered_map<string_view, int>;

namespace MathParser {

void PrintStringView(const std::string_view& str) {
    for (char c : str) {
        Log("%c", c);
    }
}

void Equation::Print() const {
    if (myEquationName.size()) {
        Log(LOG_COL_INFO "---------- ");
        PrintStringView(myEquationName);
        Log("() ----------\n" LOG_COL_RESET);
    }
    else {
        Log(LOG_COL_WARN "----- (Unnamed) -----\n" LOG_COL_RESET);
    }

    for (int i = 0; i < myNodeCount; i++) {
        const NodeGeneric& node = myNodes[i];
        node.Print();
    }

    if (myEquationName.size()) {
        Log(LOG_COL_INFO "---------- ~");
        PrintStringView(myEquationName);
        Log("() ----------\n" LOG_COL_RESET);
    }
}

void PrintParams(MapParams map) {
    Log("\n%s----------    Params    ----------%s\n", LOG_COL_WARN, LOG_COL_RESET);
    for (auto&& pair : map) {
        std::string str = std::string(pair.first);
        Log("%d : %s\n", pair.second, str.c_str());
    }
    Log("%s----------------------------------%s\n", LOG_COL_WARN, LOG_COL_RESET);
}

void Context::PrintTokens(const std::vector<TokenData>& tokens) {
    LogInfo("");
    Log("%s----------    Tokens    ----------%s\n", LOG_COL_WARN, LOG_COL_RESET);
    for (const TokenData& t : tokens) {
        char message[100] = "(Invalid)";
        switch(t.Type) {
        case Token_Invalid:
            break;
        case Token_Number:
            sprintf(message, "%-15s: %+.04f", "Number", t.NumberValue);
            break; 
        case Token_EqualSign:
            sprintf(message, "%-15s: =", "EqualSign");
            break;

        case Token_Expression:
        {
            std::string str = std::string(t.Str);
            sprintf(message, "%-15s: %s", "Expr", str.c_str());
            break;
        }
        
        case Token_Operator:
            sprintf(message, "%-15s: %c", "Operator", t.charValue);
            break;
        
        case Token_Comma:
            sprintf(message, "%-15s: ,", "Comma");
            break;

        case Token_Param_Start: 
            sprintf(message, "%-15s: %c", "Param_Start", t.charValue);
            break;
        
        case Token_Param_End:
            sprintf(message, "%-15s: %c", "Param_End", t.charValue);
            break;

        case Token_Brack_Start:
            sprintf(message, "%-15s: %c", "Brack_Start", t.charValue);
            break;

        case Token_Brack_End:
            sprintf(message, "%-15s: %c", "Brack_End", t.charValue);
            break;
        
        }

        Log("%s\n", message);
    
    }
    Log("%s----------------------------------%s\n", LOG_COL_WARN, LOG_COL_RESET);
}

void Context::PrintProperties(bool bPrintBuiltIn) {
    Log("\n%s----------    Props    ----------%s\n", LOG_COL_WARN, LOG_COL_RESET);
    

    int i = (bPrintBuiltIn ? 0 : myCustomEqStart);
    for (; i < myMaxEquations; i++)
    {
        Equation* eq = myEquations[i];
        if (!eq)
            continue;
        
        if (bPrintBuiltIn && i == myCustomEqStart) {
            Log("%s----------------------------------%s\n", LOG_COL_WARN, LOG_COL_RESET);
        }

        Log("%s\n", myStrEquations[i].c_str() );
        Log("%s" LOG_COL_RESET ", implicit: " LOG_COL_INFO "%d" LOG_COL_RESET ", explicit: " LOG_COL_INFO "%d" LOG_COL_RESET "\n", 
            (eq->Valid() ? LOG_COL_INFO "Valid" : LOG_COL_ERROR "Invalid"), 
            eq->IParamCount(), 
            eq->EParamCount()
        );

        if ( eq->Valid() && eq->IParamCount() == 0 && eq->EParamCount() == 0)
        {
            double val = eq->Evaluate(0, 0);
            Log("Value: " LOG_COL_INFO "%+.4f\n" LOG_COL_RESET, val);
        }
        Log("\n");
    }
    Log("%s----------------------------------%s\n", LOG_COL_WARN, LOG_COL_RESET);
}

//--------------------------------------------------------------------------------
//                               Equation
//--------------------------------------------------------------------------------

void Equation::ResolveEquations(Context* ctx) {
    Assert(ctx);
    for (int i = 0; i < myNodeCount; i++) {
        if (myNodes[i].type == NodeType::NodeExpression) {
            NodeExpression* node = myNodes[i].GetExpr();
            Assert(node);
            node->ResolveEquations(ctx);
        }
    }
}

void Equation::FetchProperties() {
    // myIsValid = false;
    // myIParamCount = -1;
    // myEParamCount = -1;

    int iParamCount = 0;
    int eParamCount = 0;

    for (int i = 0; i < myNodeCount; i++) {
        NodeGeneric& n = myNodes[i]; 

        if (n.type == NodeType::NodeParam) {
            NodeParam* np = n.GetParam();
            Assert(np);
            if ( np->Implicit() ) {
                iParamCount = Max(iParamCount, np->Index() + 1);
            }
            else {
                eParamCount = Max(eParamCount, np->Index() + 1);
            }
        }
        else if (n.type == NodeType::NodeExpression) {
            NodeExpression* ne = n.GetExpr();
            Assert(ne);
            ne->FetchProperties();
            
            iParamCount = Max(iParamCount, ne->GetEquation()->myIParamCount);
            //We do not compare eParamCount with ne->GetEquation()->myEParamCount, as a function call's parameters is irrelevant
            //Eg: 
            // f(a) = a+ g( a, 2*a )
            // g(b,c) = b+c
            // Here f should have only 1 eParam. But ne->GetEquation()->myEParamCount will be 2 as g() has 2 params

            for (const Equation& eq : ne->GetParams()) {
                iParamCount = Max(iParamCount, eq.myIParamCount);
                eParamCount = Max(eParamCount, eq.myEParamCount);
            }
            // eParamCount = Max(eParamCount, ne->GetEquation()->myEParamCount);
        }
    }
    
    myEParamCount = eParamCount;
    myIParamCount = iParamCount;
    Assert (myIParamCount <= 3 && myIParamCount >= 0);

    //Calculate IsValid now. The only way to do it is to simulate a Evaluate()
    std::vector<NodeValue> eParams;
    eParams.reserve(myEParamCount);
    for (int i = 0; i < myEParamCount; i++) {
        eParams.emplace_back(0.0);
    }
    NodeValue iParams[3];
    myIsValid = EvaluatePrivate(iParams, myIParamCount, eParams.data() , myEParamCount).Success;
}

double Equation::Evaluate(double x, double y)
{
    NodeValue val[2];
    val[0].SetValue(x);
    val[1].SetValue(y);
    NodeValueType ret = EvaluatePrivate(val, 2, nullptr, 0);
    Assert(ret.Success);
    return ret.Value.GetValue();
}

double Equation::Evaluate(double x, double y, double z)
{
    NodeValue val[3];
    val[0].SetValue(x);
    val[1].SetValue(y);
    val[2].SetValue(z);
    NodeValueType ret = EvaluatePrivate(val, 3, nullptr, 0);
    Assert(ret.Success);
    return ret.Value.GetValue();
}

NodeValueType Equation::EvaluatePrivate(NodeValue* iParams, int iSize, NodeValue* eParams, int eSize)
{
    //Todo: store this on the stack instead
    std::stack<NodeValue> stackValues;
    for (int i = 0; i < myNodeCount; i++) {
        NodeGeneric& node = myNodes[i];
        switch (node.type) {
            case NodeType::NodeValue:
            {
                stackValues.push( *node.GetValue() );
                break;
            }
            case NodeType::NodeParam:
            {
                NodeParam* np = node.GetParam();
                Assert(np);
                if (np->Implicit()) {
                    if (np->Index() >= iSize)
                        return NodeValueType(false);
                    
                    stackValues.push( iParams[np->Index()] );
                }
                else {
                    if (np->Index() >= eSize)
                        return NodeValueType(false);
                    stackValues.push( eParams[np->Index()] );
                }
                break;
            }
            case NodeType::NodeOperator:
            {
                //Todo: calculate automatically pops the stack because operators might not always be binary and take two operands
                NodeValueType res = node.GetOp()->Calculate(stackValues);
                if (!res.Success)
                    return res;
                stackValues.push(res.Value);
                break;
            }
            case NodeType::NodeExpression:
            {
                NodeValueType res = node.GetExpr()->Calculate(iParams, iSize, eParams, eSize);
                if (!res.Success)
                    return res;
                stackValues.push(res.Value);
                break;
            }

            default:
                Assert(false && "Unknown type");
                return NodeValueType(false);
                break;
        } //End of switch
    } //End of for loop

    if (stackValues.size() != 1)
        return NodeValueType(false);
    
    double val = stackValues.top().GetValue();
    return NodeValueType( true, val );
}

//--------------------------------------------------------------------------------
//                               Context
//--------------------------------------------------------------------------------

Context::Context()
{
    myCount = 0;

    for (int i = 0; i < myMaxEquations; i++) {
        myEquations[i] = nullptr;
    }

    AddInbuiltEqs();
}

Context::~Context() {
    ClearPrivate();
}

void Context::AddInbuiltEqs() {
    
    //sin
    {
        Equation* eq = new Equation;
        eq->SetName("sin");
        eq->PushNode( NodeParam(0, false) );
        eq->PushNode( NodeOperator(NodeOperator::OP_SIN) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: sin(x)";
        myCount++;
    }

    //cos
    {
        Equation* eq = new Equation;
        eq->SetName("cos");
        eq->PushNode( NodeParam(0, false) );
        eq->PushNode( NodeOperator(NodeOperator::OP_COS) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: cos(x)";
        myCount++;
    }
    //tan
    {
        Equation* eq = new Equation;
        eq->SetName("tan");
        eq->PushNode( NodeParam(0, false) );
        eq->PushNode( NodeOperator(NodeOperator::OP_TAN) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: tan(x)";
        myCount++;
    }
    
    //sqrt
    {
        Equation* eq = new Equation;
        eq->SetName("sqrt");
        eq->PushNode( NodeParam(0, false) );
        eq->PushNode( NodeOperator(NodeOperator::OP_SQRT) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: sqrt(x)";
        myCount++;
    }
    //exp
    {
        Equation* eq = new Equation;
        eq->SetName("exp");
        eq->PushNode( NodeParam(0, false) );
        eq->PushNode( NodeOperator(NodeOperator::OP_EXP) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: exp(x)";
        myCount++;
    }

    //Constants
    {
        Equation* eq = new Equation;
        eq->SetName("pi");
        eq->PushNode( NodeValue( glm::pi<double>() ) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: pi";
        myCount++;
    }

    {
        Equation* eq = new Equation;
        eq->SetName("e");
        eq->PushNode( NodeValue( glm::exp(1) ) );
        
        Assert(myCount < myMaxEquations);
        myEquations[myCount] = eq;
        myStrEquations[myCount] = "Inbuilt: e";
        myCount++;
    }

    myCustomEqStart = myCount;
}

void Context::Clear() {
    ClearPrivate();
    AddInbuiltEqs();
}
void Context::ClearPrivate() {
    for (int i = 0; i < myMaxEquations; i++) {
        Equation* eq = myEquations[i];
        if (eq) {
            delete eq;
        }
        myEquations[i] = nullptr;
        myStrEquations[i] = "";
    }
    myCount = 0;
}

Context& Context::operator= (Context&& other) {
    int maxCount = Max(myCount, other.myCount);
    for (int i = 0; i < maxCount; i++) {
        std::swap(myStrEquations[i], other.myStrEquations[i]);
        std::swap(myEquations[i], other.myEquations[i]);
    }
    std::swap(myCount, other.myCount);

    return *this;
}

bool Context::Resolve() {
    for (int i = 0; i < myCount; i++) {
        Assert(myEquations[i]);
        if (myEquations[i]) {
            myEquations[i]->ResolveEquations(this);
        }
    }
    
    for (int i = 0; i < myCount; i++) {
        Assert(myEquations[i]);
        if (myEquations[i]) {
            myEquations[i]->FetchProperties();
        }
    }

    return true;
}

Equation* Context::FindEquation(const std::string_view& str) {
    for (int i = 0; i < myCount; i++) {
        Assert(myEquations[i]);

        if (myEquations[i] && myEquations[i]->Name() == str) {
            return myEquations[i];
        }
    }
    return nullptr;
}

Equation* Context::FindEquationIndex(int index) {
    Assert(index < myMaxEquations);
    if (index < myMaxEquations)
        return myEquations[index];
    return nullptr;
}

bool Context::LoadFromFile(const std::string& str) {
    std::ifstream file;
    file.open(str.c_str());
    if (!file.is_open())
    {
        return false;
    }

    char buff[300];
    while (!file.eof())
    {
        file.getline(buff, 300);
         if ( buff[0] == '\0' || buff[0] == '#' )
             continue;

        // LogTrace("Line: %s", buff);
        AddEquation(buff);
    }
    
    if ( !Resolve()) {
        return false;
    }

    return true;
}

bool Context::RunAllTests() {
    Context c;
    bool bVal = c.RunTest_InfixToToken();
    return bVal;
}

bool Context::RunTest_InfixToToken() {
    return true;
}

} //End of namespace MathParser