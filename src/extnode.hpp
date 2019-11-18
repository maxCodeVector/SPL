#ifndef __EXTNODE__
#define __EXTNODE__

#include <string>
#include <list>
#include "deliver.hpp"

using namespace std;

class Location {
private:
    int lineNo;
public:
    Location(int line, int col) {
        this->lineNo = line;
    }

    string toString() {
        return "line " + to_string(lineNo);
    }
};

class ConstantEntry {
    string name;
    string value;
};

class ConstantTable {
    list<ConstantEntry> entries;
};

class Scope;

enum NodeType {
    VAR,
    FUNC,
    OTHER
};

enum DataType{
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    STRUCT_TYPE,
    OTHER_TYPE
};

class BaseNode {

public:
    enum NodeType flag=OTHER;
    BaseNode *next;
    Location *loc;

    virtual void setScope(Scope *scope) {}

    virtual Location *location() {
        return loc;
    }

    virtual void setNext(AttrNode *extDef);
};

class Entity : public BaseNode{
public:
    string type;
    string value;
    bool isRefered = false;

    virtual string &getName()=0;

    void refere(){
        isRefered = true;
    }
};

class VariableType : public BaseNode {
    enum DataType type;
public:
    explicit VariableType(DataType type){
        this->type = type;
    }
};
VariableType* getVariableType(string &name);


class DefinedVariable;
class Struct: VariableType{
public:
    string typeName;
    list<DefinedVariable> members;
};

class Exp;
class DefinedVariable : public Entity {
private:
    VariableType* type;
    string id;
    Exp* value;
    list<int > array;
public:

    explicit DefinedVariable(AttrNode *varDec);

    void setType(AttrNode *spec);
    void addDimension(AttrNode *dim);
    void setExp(AttrNode* exp){
        this->value = (Exp*)exp->baseNode;
    }

    string & getName() override {
        return this->id;
    }


};


enum Operator{
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    NOT_OP,
    INVOKE,
    ASSIGN_OP,
    AND_OP,
    OR_OP,
    LT_OP,
    LE_OP,
    GT_OP,
    GE_OP,
    NE_OP,
    EQ_OP
};

class Exp:public BaseNode{
protected:
    enum Operator operatorType;
    enum DataType type;
    string value;
public:
    Exp(AttrNode* terminal, DataType dataType);
    Exp(){}
};

class Statement:public BaseNode{
protected:
    Exp *exp;
public:
    explicit Statement(AttrNode* exp);
};

class Body : public BaseNode{
public:
    list<DefinedVariable> vars;
    list<Statement*> statements;
    Body(AttrNode* defList, AttrNode* stmtList);

};

class IfStatement:public Statement{
public:
    Statement* ifbody;
    Statement* elseBody;
    IfStatement(AttrNode* exp, AttrNode* ifNode, AttrNode* elseNode):Statement(exp){
        ifbody  = (Statement*)ifNode->baseNode;
        elseBody = (Statement*)elseNode->baseNode;
    }
    IfStatement(AttrNode* exp, AttrNode* ifNode):Statement(exp){
        ifbody  = (Statement*)ifNode->baseNode;
    }
};

class WhileStatement:public Statement{
public:
    Statement* loop;
    WhileStatement(AttrNode* exp, AttrNode* loopNode):Statement(exp){
        loop = (Statement*)loopNode->baseNode;
    }
};

class ReturnStatement:public Statement{
public:
    explicit ReturnStatement(AttrNode* exp):Statement(exp){}
};


class DefinedFunction : public Entity {
private:
    VariableType* returnType;
    string id;
    list<DefinedVariable> parameters;
    Body* functionBody;
    void parseParameters(AttrNode* paraList);

public:
    DefinedFunction(AttrNode *functionID, AttrNode *paraList);
    explicit DefinedFunction(AttrNode *functionID);


    list<DefinedVariable>& getParameters();
    void setReturnType(AttrNode* type);

    Body *getBody();
    void setBody(AttrNode* body){
        this->functionBody = (Body*)body->baseNode;
    }

    string &getName() override {
        return this->id;
    }

};

class BinaryExp:public Exp{
public:
    Exp* left;
    Exp* right;
    BinaryExp(AttrNode * le, AttrNode * rig, Operator operatorType);

};

class SingleExp:public Exp{
public:
    Exp* operated;
    SingleExp(AttrNode* operated, Operator operatorType);

};

class Args: public BaseNode{
public:
    list<Exp*> args;
};

class InvokeExp:public Exp{
public:
    string functionName;
    Args *args;
    explicit InvokeExp(AttrNode* invoker);
    InvokeExp(AttrNode* invoker, AttrNode* args);
};

#endif