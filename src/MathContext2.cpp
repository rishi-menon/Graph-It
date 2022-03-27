
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

bool Context::AddEquation(const std::string& str) {

    if (str.empty() || str.at(0) == '#')
        return false;

    std::string strError;
    // std::vector<TokenData> infix;
    
    Assert(myCount < myMaxEquations );
    std::string* pStr = &myStrEquations[myCount]; 
    *pStr = str;
    
    Equation eq(this);
    if (!AddEquationPrivate(*pStr, &strError, &eq)) {
        LogInfo("Failed to parse equation: %s", str.c_str());
        if (strError.size())
        {
            LogInfo("Reason: %s", strError.c_str());
        }
        return false;
    }
    //eq.Print();
    myEquations[myCount] = new Equation( std::move(eq) );
    myCount++;
    return true;
}



bool Context::AddEquationPrivate(const std::string& strEquation, std::string* outError, Equation* eq) {
    std::vector<TokenData> infixTokens;
    if (!ParseInfixToTokens(strEquation, infixTokens, outError)) {
        return false;
    }

    //PrintTokens(infixTokens);
    if ( !InfixToPostFix(infixTokens, outError, eq) ) {
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


//Tries to parse a function name. If successful, outNewIndex will point to the equal sign after the function name
bool TryParseFunctionName(TokenData* infix, int size, std::string* outError, int& outNewIndex, string_view& outFuncName, std::unordered_map<string_view, int>& outMapParams)
{
    #define EXIT_MSG(MSG) \
    do { \
        if (outError) *outError = MSG; \
        return false; \
    } while (false)

    #define CHECK_SIZE(COUNT) \
    do { \
        if (COUNT >= size) { \
            EXIT_MSG("Expected more tokens"); \
        } \
    } while (false)

    int index = 0;
    CHECK_SIZE(index);

    if (infix[index].Type != Token_Expression)
        EXIT_MSG("Expected expression name if we have an equal sign");
    
    outFuncName = infix[index].Str;
    index++;
    CHECK_SIZE(index);
    if (infix[index].Type == Token_EqualSign) {
        //Constant, not a function
        outNewIndex = index;
        return true;
    }

    //In this case we must have parameters now
    if ( infix[index].Type != Token_Param_Start )
        EXIT_MSG("Expected parameters after function name");
    index++; CHECK_SIZE(index);

    int paramIndex = 0;
    while ( infix[index].Type != Token_Param_End ) {
        //Here we should only have a comma seperated list of identifiers (parameter names)
        if (infix[index].Type != Token_Expression)
            EXIT_MSG("Expected param name");
        TokenData& expr = infix[index];
        index++; CHECK_SIZE(index);

        //The last param doesnt have a comma. Allowed values here are only comma or PARAM_END
        if (infix[index].Type != Token_Comma && infix[index].Type != Token_Param_End)
            EXIT_MSG("Expected comma after param name");
        
        if (infix[index].Type == Token_Comma)
        {
            index++; CHECK_SIZE(index);
        }
        
        if (expr.Str == "x" || expr.Str == "y" || expr.Str == "z")
            EXIT_MSG("x, y, z are reserved keywords and cannot be the name of a parameter");
        
        outMapParams[expr.Str] = paramIndex;
        paramIndex++;
    }
    Assert (infix[index].Type == Token_Param_End);
    index++; CHECK_SIZE(index);
    if (infix[index].Type != Token_EqualSign)
        EXIT_MSG("Expected equal sign after function params");
    
    outNewIndex = index;

    #undef EXIT_MSG
    #undef CHECK_SIZE
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////




struct InputParams
{
    Context* ctx;
    TokenData* infix;
    int size;
    std::string* outError;

    //These variables mutate
    int startIndex;
};

bool TryProcessExpression(InputParams& in, MapParams* mapParams, int* outIndex, NodeGeneric* outNode);
bool TryProcessEquation(InputParams& in, MapParams* mapParams, int* outIndex, Equation* outEq);


bool TryProcessExpression(InputParams& in, MapParams* mapParams, int* outIndex, NodeGeneric* outNode)
{
    #define EXIT_MSG(MSG) \
    do { \
        if (in.outError && MSG) *in.outError = MSG; \
        return false; \
    } while (false)

    #define CHECK_SIZE(COUNT) \
    do { \
        if (COUNT >= in.size) { \
            EXIT_MSG("Expected more tokens"); \
        } \
    } while (false)

    CHECK_SIZE(in.startIndex);
    TokenData& expr = in.infix[in.startIndex];

    Assert(expr.Type == Token_Expression);
    
    if ( (in.startIndex+1) < in.size && in.infix[in.startIndex+1].Type == Token_Param_Start)
    {
        //Process this as a function
        int index = in.startIndex+2;
        CHECK_SIZE(index);
        std::vector<Equation> eqs;
        eqs.reserve(10);
        while ( in.infix[index].Type != Token_Param_End )
        {
            int newIndex;
            Equation eq(in.ctx);
            in.startIndex = index;
            if ( !TryProcessEquation(in, mapParams, &newIndex, &eq) )
                return false;
            
            index = newIndex;
            CHECK_SIZE(index);

            //The last param doesnt have a comma. Allowed values here are only comma or PARAM_END
            if (in.infix[index].Type != Token_Comma && in.infix[index].Type != Token_Param_End)
                EXIT_MSG("Expected comma after param name");
            
            if (in.infix[index].Type == Token_Comma) {
                index++; CHECK_SIZE(index);
            }
            eqs.push_back( std::move(eq) );
        }
        Assert(in.infix[index].Type == Token_Param_End);
        index++;
        Assert(outNode);
        outNode->type = NodeType::NodeExpression;
        outNode->data = NodeExpression(expr.Str, std::move(eqs) );

        *outIndex = index;
    }
    else 
    {
        //This is either a parameter or a const expression (variable)
        Assert(outNode);
        
        if (expr.Str == "x" || expr.Str == "y" || expr.Str == "z") {
            //Implicit parameter. x:0, y:1, z:2 
            int paramNum =  tolower( expr.Str.at(0) ) - 'x';
            outNode->type = NodeType::NodeParam;
            outNode->data = NodeParam(paramNum, true);
        }
        else
        {
            Assert (mapParams);
            auto it = mapParams->find( expr.Str );
            if (it != mapParams->end()) {
                //This is a parameter
                int paramNum = it->second;
                outNode->type = NodeType::NodeParam;
                outNode->data = NodeParam(paramNum, false);
            }
            else
            {
                //This is a const expression
                outNode->type = NodeType::NodeExpression;
                outNode->data = NodeExpression(expr.Str);
            }
        }

        *outIndex = in.startIndex + 1;
    }

    #undef EXIT_MSG
    #undef CHECK_SIZE

    return true;
}




bool TryProcessEquation(InputParams& in, MapParams* mapParams, int* outIndex, Equation* outEq)
{
    #define EXIT_MSG(MSG) \
    do { \
        if (in.outError && MSG) *in.outError = MSG; \
        return false; \
    } while (false)

    #define CHECK_SIZE(COUNT) \
    do { \
        if (COUNT >= in.size) { \
            EXIT_MSG("Expected more tokens"); \
        } \
    } while (false)

    //Time to convert expression to postfix
    // See: https://raj457036.github.io/Simple-Tools/prefixAndPostfixConvertor.html
    /*
    Step 1: Add ")" to the end of the infix expression
    Step 2: Push ( onto the stack
    Step 3: Repeat until each character in the infix notation is scanned
        IF a ( is encountered, push it on the stack
        
        IF an operand (whether a digit or a character) is encountered, add it postfix expression.
        
        IF a ")" is encountered, then
        a. Repeatedly pop from stack and add it to the postfix expression until a "(" is encountered.
        b. Discard the "(".That is, remove the ( from stack and do not add it to the postfix expression
        
        IF an operator O is encountered, then
        a. Repeatedly pop from stack and add each operator ( popped from the stack) to the postfix expression which has the same precedence or a higher precedence than O
        b. Push the operator to the stack
    [END OF IF]
    Step 4: Repeatedly pop from the stack and add it to the postfix expression until the stack is empty
    Step 5: EXIT
    */

    int index = in.startIndex;
    CHECK_SIZE(index);

    auto CalculatePrecedence = [](const TokenData& d) -> int {
        Assert(d.Type == Token_Operator);
        if (d.Type != Token_Operator)
            return -2;
        char c = d.charValue;

        //Increasing order of priority
        if (c == '+' || c == '-')
            return 0;
        if (c == '*' || c == '/')
            return 1;
        if (c == '^')
            return 2;
        Assert("Unknown operator type");
        return -1;
        
    };

    std::stack<TokenData> stackOperators;
    {
        TokenData td;
        td.Type = Token_Brack_Start;
        td.charValue = '(';
        stackOperators.push(td);
    }

    auto ProcessClosingBracket = [&]() {
        //Keep popping operators till we see a opening bracket on the stack and then discard that
        while (true) {
            if (stackOperators.empty()) {
                Assert(false && "Dont know why this happened. Probably because typed a closing bracket before an opening bracket");
                EXIT_MSG("Something went wrong while parsing");
            }

            TokenData data = stackOperators.top();
            stackOperators.pop();
            if (data.Type == Token_Brack_Start)
                break;
            Assert(data.Type == Token_Operator && "We pushed something else to the operator stack");
            //Add operator to the equation 
            NodeGeneric gen;
            gen.type = NodeType::NodeOperator;
            gen.data = NodeOperator(data.charValue);
            Assert(outEq);
            outEq->PushNode(gen);
        }
        return true;
    };




    Assert(outEq);
    bool bRun = true;
    while ( index < in.size && bRun )
    {
        TokenData& d = in.infix[index];
        switch (d.Type) {
            case Token_Brack_Start:
            {
                stackOperators.push(d);
                index++;
                break;
            }
            case Token_Brack_End:
            {
                if ( !ProcessClosingBracket() )
                    return false;
                index++;
                break;
            }
            case Token_Operator:
            {
                while (true) {
                    if (stackOperators.empty()) {
                        Assert(false && "Dont know why this happened");
                        EXIT_MSG("Something went wrong while parsing");
                    }
                    const TokenData& cur = stackOperators.top();
                    if (cur.Type == Token_Brack_Start)
                        break;
                    
                    Assert(cur.Type == Token_Operator && "We pushed something else to the operator stack");
                    
                    if ( CalculatePrecedence(d) > CalculatePrecedence(cur) )
                    {
                        break;
                    }
                    //Add operator to the equation 
                    NodeGeneric gen;
                    gen.type = NodeType::NodeOperator;
                    gen.data = NodeOperator(cur.charValue);
                    
                    outEq->PushNode(gen);

                    stackOperators.pop();
                }
                stackOperators.push(d);
                index++;
                break;
            }
            
            //Rishi to do: Write code for inserting expressions and functions and numbers to the equation
            case Token_Number:
            {
                //Add number to the equation 
                NodeGeneric gen;
                gen.type = NodeType::NodeValue;
                gen.data = NodeValue(d.NumberValue);
                outEq->PushNode(gen);
                index++;
                break;
            }
            case Token_Expression:
            {
                //This can either be a parameter or a const expression (user variable) or a function call
                int newOutIndex;
                NodeGeneric gen;
                in.startIndex = index;
                if ( !TryProcessExpression(in, mapParams, &newOutIndex, &gen) )
                    return false;
                index = newOutIndex;
                outEq->PushNode(gen);
                break;
            }

            case Token_EqualSign:
            case Token_Param_End:
            case Token_Comma:
            {
                bRun = false;
                break;
            }

            default:
            {
                // Assert(false);
                EXIT_MSG("Unexpected token type while parsing equation");
            }
        }
    }

    //Process the remaining operators as we inserted a '(' at the beginning
    if ( !ProcessClosingBracket() )
        return false;

    *outIndex = index;

    #undef EXIT_MSG
    #undef CHECK_SIZE
    
    return true;

}


bool Context::InfixToPostFix(std::vector<TokenData>& infix, std::string* outError, Equation* outEq) 
{
    #define EXIT_MSG(MSG) \
    do { \
        if (outError && MSG) *outError = MSG; \
        return false; \
    } while (false)

    #define CHECK_SIZE(COUNT) \
    do { \
        if (COUNT >= infix.size()) { \
            EXIT_MSG("Expected more tokens"); \
        } \
    } while (false)

    int equalSignCount = 0;
    
    for (int i = 0; i < infix.size(); i++) {
        TokenData& d = infix[i]; 
        if (d.Type == Token_EqualSign)
            equalSignCount++;
    }

    if (equalSignCount > 1)
        EXIT_MSG("Multiple equal signs");

    int index = 0;
    CHECK_SIZE(index);

    string_view equationName;
    MapParams mapParams;
    
    //Fetch name and params
    if (equalSignCount == 1) {
        int newIndex;
        if (! TryParseFunctionName(infix.data(), infix.size(), outError, newIndex, equationName, mapParams) )
        {
            EXIT_MSG("Could not parse function name");
        }
        index = newIndex;
        Assert(infix[index].Type == Token_EqualSign);
        index++; CHECK_SIZE(index);
    }

    //PrintParams(mapParams);

    Assert (outEq);
    outEq->SetName(equationName);

    InputParams in;
    in.ctx = this;
    in.infix = infix.data();
    in.size = infix.size();
    in.outError = outError;
    in.startIndex = index;
    int newIndex;
    if ( !TryProcessEquation(in, &mapParams, &newIndex, outEq) ) {
        return false;
    }
    

#undef EXIT_MSG
#undef CHECK_SIZE

    return true;
}

bool Context::ParseInfixToTokens(const std::string& strEquation, std::vector<TokenData>& infixTokens, std::string* outError)
{
    if (strEquation.empty())
    {
        if (outError) *outError = "Equation was an empty string";
        return false;
    }

    TokenType prevToken = Token_Invalid;

    int index = 0;
    int numParam = 0;
    int numBracket = 0;

    auto ParseNum = [&]() {
        TokenData d;
        Assert(index < strEquation.length());
        double val;
        int count;
        const char* strStart = &strEquation[index]; 
        int res = sscanf( strStart, "%lf %n", &val, &count);
        if (res == 1) {
            d.Type = Token_Number;
            d.NumberValue = val;
            infixTokens.push_back(d);
            
            index += count;
            prevToken = Token_Number;
            return true;
        }
        return false;
    };

    for (index = 0; index < strEquation.size();) {
        char c = strEquation.at(index);

        if ( isdigit(c) || c == '.') {
            if ( !ParseNum() ) {
                if (outError) *outError = "Cannot parse token: " + std::string( &strEquation[index], 3 );
                return false;
            }
        }
        else if (c == ' ') {
            index++;
            continue;
        }
        else if ( isalpha(c) || c == '_') {
            const char* strStart = &strEquation[index];
            int startIndex = index;
            while ( index < strEquation.length() ) {
                char curChar = strEquation.at(index);
                //Only the following characters are allowed in an identifier name (like a function name or a constant name)
                if (isalpha(curChar) || isdigit(curChar) || curChar == '_') {
                    index++;
                }
                else
                    break;
            }

            if (startIndex != index) {
                TokenData d;
                d.Type = Token_Expression;
                d.Str = string_view(strStart, index-startIndex);
                infixTokens.push_back(d);
                prevToken = Token_Expression;
            }
            else {
                Assert(false && "Something weird happened");
                return false;
            }

        }
        else if (c == ',') {
            //Single character token
            TokenData d;
            d.Type = Token_Comma;
            infixTokens.push_back(d);
            prevToken = Token_Comma;
            index++;
        }
        else if ( c == '(' ) {
            if (prevToken == Token_Expression) {
                //Treat this as the start of a function call
                TokenData d;
                d.Type = Token_Param_Start;
                d.charValue = c;
                infixTokens.push_back(d);
                prevToken = Token_Param_Start;
                index++;

                numParam++;
            }
            else {
                //Treat this as a regular bracket
                TokenData d;
                d.Type = Token_Brack_Start;
                d.charValue = c;
                infixTokens.push_back(d);
                prevToken = Token_Brack_Start;
                index++;

                numBracket++;
            }
        }
        else if (c == ')') {
            //This might either be a regular bracket end or a function bracket end
            if (numBracket > 0) {
                //Interpret this as a closing bracket
                TokenData d;
                d.Type = Token_Brack_End;
                d.charValue = c;
                infixTokens.push_back(d);
                prevToken = Token_Brack_End;
                index++;
                numBracket--;
            }
            else if (numParam > 0) {
                TokenData d;
                d.Type = Token_Param_End;
                d.charValue = c;
                infixTokens.push_back(d);
                prevToken = Token_Param_End;
                index++;
                numParam--;
            }
            else {
                if (outError) *outError = "Too many closing ) brackets";
                return false;
            }
        }
        else if (c == '[' || c == '{')
        {
            //Treat as regular bracket for the time being
            TokenData d;
            d.Type = Token_Brack_Start;
            d.charValue = c;
            infixTokens.push_back(d);
            prevToken = Token_Brack_Start;
            index++;
        }
        else if (c == ']' || c == '}')
        {
            //Treat as regular bracket for the time being
            TokenData d;
            d.Type = Token_Brack_End;
            d.charValue = c;
            infixTokens.push_back(d);
            prevToken = Token_Brack_End;
            index++;
        }
        else if (c == '+' || c == '-') {
            // This could either be a unary operator for indicating positive/negative numbers, or a binary operator
            // for an operation.
            if (prevToken == Token_Invalid || 
                prevToken == Token_Operator || 
                prevToken == Token_Comma || 
                prevToken == Token_Param_Start || 
                prevToken == Token_Brack_Start
                )
            {
                //Treat as unary operator
                if ( !ParseNum() ) {
                    if (outError) *outError = "Cannot parse token: " + std::string( &strEquation[index], 3 );
                    return false;
                }
            }
            else
            {
                //Treat as binary operator
                TokenData d;
                d.Type = Token_Operator;
                d.charValue = c;
                infixTokens.push_back(d);
                prevToken = Token_Operator;
                index++;
            }
        }
        else if (c == '*' || c == '/' || c == '^')
        {
            TokenData d;
            d.Type = Token_Operator;
            d.charValue = c;
            infixTokens.push_back(d);
            prevToken = Token_Operator;
            index++;
        }
        else if (c == '=') {
            TokenData d;
            d.Type = Token_EqualSign;
            d.charValue = c;
            infixTokens.push_back(d);
            prevToken = Token_EqualSign;
            index++;
        }
        else {
            LogWarn("Unknown Type: %c", (char)(c) );
            Assert(false);
        }
    }   //End of for loop

    if (numBracket != 0 || numParam != 0) {
        if (outError) *outError = "Bracket ( mimatch";
        return false;
    }

    return true;
}

} //End of namespace