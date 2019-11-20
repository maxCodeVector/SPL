//
// Created by hya on 11/20/19.
//

#ifndef SPL_EXPRESSION_H
#define SPL_EXPRESSION_H

#include "extnode.h"

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
    DataType getType(){
        return type;
    };
    void setReferenceVar(Entity* entity)
    {
        referenceVar=entity;
    }
    virtual Error* checkType(){ return nullptr;}
    virtual Error * checkReference(Scope* scope);
    virtual bool isLeftValue();
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
    Error* checkType() override ;

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


#endif //SPL_EXPRESSION_H
