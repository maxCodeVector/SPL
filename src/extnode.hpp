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


class Entity {
public:
    string name;
    string type;
    string value;
    Location *location;

    Entity(string &name) {
        this->name = name;
    }
};

class ConstantEntry {
    string name;
    string value;
};

class ConstantTable {
    list<ConstantEntry> entries;
};


class AST;// defined for program
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
    enum NodeType flag;
    BaseNode *next;
    Location *loc;

    virtual void setScope(Scope &scope) {}

    virtual Location *location() {
        return loc;
    }

    virtual void setNext(AttrNode *extDef);
};

class VariableType : public BaseNode {
    DataType type;
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

class DefinedVariable : public BaseNode {
private:
    VariableType* type{};
    string id;
    string value;
    list<int > array;
public:
    DefinedVariable(AttrNode *spec, AttrNode *decList);

    explicit DefinedVariable(AttrNode *varDec);

    void setType(AttrNode *spec);
    void addDimension(AttrNode *dim);

    string &name() {
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
    Operator operatorType;
    DataType type;
    string value;
public:
    Exp(AttrNode* terminal, DataType dataType);
    Exp(){}
};

class Statement:public BaseNode{
protected:
    Exp exp;
};

class Body : public BaseNode{
public:
    list<DefinedVariable> vars;
    list<Statement> statements;

};

class IfStatement:public Statement{
public:
    Body ifbody;
    Body elseBody;
};

class WhileStatement:public Statement{
public:
    Body loop;
};

class ReturnStatement:public Statement{

};


class DefinedFunction : public BaseNode {
private:
    VariableType* returnType;
    string id;
    list<DefinedVariable> parameters;
    Body functionBody;
    void parseParameters(AttrNode* paraList);

public:
    DefinedFunction(AttrNode *functionID, AttrNode *paraList);
    explicit DefinedFunction(AttrNode *functionID);


    list<DefinedVariable>& getParameters();
    void setReturnType(AttrNode* type);

    AST *getBody();

    string &name() {
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
    list<Exp> args;
};

class InvokeExp:public Exp{
public:
    string functionName;
    Args *args;
    explicit InvokeExp(AttrNode* invoker);
    InvokeExp(AttrNode* invoker, AttrNode* args);
};

#endif