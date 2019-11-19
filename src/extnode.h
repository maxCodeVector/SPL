#ifndef __EXTNODE__
#define __EXTNODE__

#include "includes.h"
#include "parse/deliver.h"

class Scope;

class BaseNode {
private:
    Location *loc;
    bool loc_alloc;// mark is it allocate Location* loc

public:
    enum NodeType flag=OTHER;
    BaseNode *next;

    virtual void setScope(Scope *scope) {};

    Location *getLocation() {
        return loc;
    }

    void setLocation(Location* location) {
        this->loc = location;
        loc_alloc = false;
    }

    void setLocation(int lineNo, int colNo) {
        this->loc = new Location(lineNo, colNo);
        loc_alloc = true;
    }

    virtual void setNext(AttrNode *extDef);
    ~BaseNode(){
        if(loc_alloc)
            delete(loc);
    }

};

class Entity : public BaseNode{
public:
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
    DataType getType(){
        return type;
    }
};
VariableType* getVariableType(string &name);


class DefinedVariable;
class Struct: public VariableType{
public:
    string typeName;
    list<DefinedVariable*> members;
    explicit Struct(AttrNode* name);
    Struct(AttrNode* name, AttrNode* defList);
    ~Struct();
    string& getTypeName(){
        return typeName;
    }
};

class DeclaredTypeVariable:public Entity{
    VariableType* type;
    string baseTypeName = "baseType";
public:
    DeclaredTypeVariable(AttrNode* spec);
    string& getName() override {
        if(type->getType()==DataType::STRUCT_TYPE){
            return ((Struct*)type)->getTypeName();
        }
        return baseTypeName;
    }
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
    Exp* getValue(){
        return value;
    };

    string & getName() override {
        return this->id;
    }

    ~DefinedVariable();

};


class Exp:public BaseNode{
protected:
    enum Operator operatorType;
    enum DataType type;
    string value;//expression value if it is ID or int:float:char
    Entity* referenceVar;
public:
    Exp(AttrNode* terminal, DataType dataType);
    Exp(){}
    string& getName(){
        return value;
    }
    void setReferenceVar(Entity* entity)
    {
        referenceVar=entity;
    }
    virtual void checkType(){};
    virtual Error * checkReference(Scope* scope);
};

class Statement:public BaseNode{
protected:
    Exp *exp;
public:
    explicit Statement(AttrNode* exp);
    ~Statement(){
        delete(exp);
    }
    Exp* getExpression(){
        return exp;
    }
};

class BreakStatement{
    virtual bool checkBreak() = 0;
};

class ContinueStatement{
    virtual bool checkContinue() = 0;
};


class Body : public BaseNode, BreakStatement, ContinueStatement{
public:
    list<DefinedVariable*> vars;
    list<Statement*> statements;
    Body(AttrNode* defList, AttrNode* stmtList);
    ~Body(){
        free_all(vars);
        free_all(statements);
    }
    bool checkContinue() override { return true;};
    bool checkBreak() override { return true;};

};

class IfStatement:public Statement{
public:
    Body* ifBody;
    Body* elseBody;
    IfStatement(AttrNode* exp, AttrNode* ifNode, AttrNode* elseNode):Statement(exp){
        ifBody  = (Body*)ifNode->baseNode;
        elseBody = (Body*)elseNode->baseNode;
    }
    IfStatement(AttrNode* exp, AttrNode* ifNode):Statement(exp){
        ifBody  = (Body*)ifNode->baseNode;
    }
    ~IfStatement(){
        delete(ifBody);
        delete(elseBody);
    }
};

class WhileStatement:public Statement{
public:
    Body* loop;
    WhileStatement(AttrNode* exp, AttrNode* loopNode):Statement(exp){
        loop = (Body*)loopNode->baseNode;
    }
    ~WhileStatement(){
        delete(loop);
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
    list<DefinedVariable*> parameters;
    Body* functionBody;
    void parseParameters(AttrNode* paraList);

public:
    DefinedFunction(AttrNode *functionID, AttrNode *paraList);
    explicit DefinedFunction(AttrNode *functionID);
    ~DefinedFunction(){
        delete(returnType);
        delete(functionBody);
        free_all(parameters);
    }


    list<DefinedVariable*>& getParameters();
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
    ~BinaryExp(){
        delete(left);
        delete(right);
    }
    Error * checkReference(Scope* scope) override;

};

class UnaryExp: public Exp{
public:
    Exp* operand;
    UnaryExp(AttrNode* operated, Operator operatorType);
    ~UnaryExp(){
        delete(operand);
    }
    Error * checkReference(Scope* scope) override;

};

class GetAttributeExp: public Exp{
    string attrName;
public:
    Exp* object;
    GetAttributeExp(AttrNode* operated, string& attributeName);
    ~GetAttributeExp(){
        delete(object);
    }
    Error * checkReference(Scope* scope) override{ return nullptr;}

};


class Args: public BaseNode{
public:
    list<Exp*> args;
    ~Args(){
        free_all(args);
    }
};

class InvokeExp:public Exp{
public:
    string functionName;
    Args *args;
    explicit InvokeExp(AttrNode* invoker);
    InvokeExp(AttrNode* invoker, AttrNode* args);
    ~InvokeExp(){
        delete(args);
    }
    Error * checkReference(Scope* scope) override;
};

#endif