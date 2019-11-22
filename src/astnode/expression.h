//
// Created by hya on 11/20/19.
//

#ifndef SPL_EXPRESSION_H
#define SPL_EXPRESSION_H

#include "extnode.h"

class DereferenceChecker;
class Exp:public BaseNode{
protected:
    enum Operator operatorType;
    VariableType* type;
    string value;//expression value if it is ID or int:float:char
    Entity* referenceVar;
    int dimension;// used for array, means which dimension current is in
public:
    Exp(AttrNode* terminal, DataType dataType);
    Exp(){}
    virtual ~Exp(){
        delete(type);
    }
    string& getName(){
        return value;
    }
    VariableType* getType(){
        return type;
    };
    void setReferenceVar(Entity* entity)
    {
        referenceVar=entity;
    }
    virtual Error* checkType(){ return nullptr;}
    virtual Error * checkReference(Scope* scope);
    virtual bool isLeftValue();
    virtual bool isArray();
    virtual void acceptDereferenceCheck(DereferenceChecker* checker){}
};


class BinaryExp:public Exp{
public:
    Exp* left;
    Exp* right;
    BinaryExp(AttrNode * le, AttrNode * rig, Operator operatorType);
    ~BinaryExp() override{
        delete(left);
        delete(right);
    }
    Error * checkReference(Scope* scope) override;
    Error* checkType() override ;
    void acceptDereferenceCheck(DereferenceChecker *checker) override;

};

class UnaryExp: public Exp{
public:
    Exp* operand;
    UnaryExp(AttrNode* operated, Operator operatorType);
    ~UnaryExp() override{
        delete(operand);
    }
    Error * checkReference(Scope* scope) override;

};

class GetAttributeExp: public Exp{
    string attrName;
public:
    Exp* object;
    GetAttributeExp(AttrNode* operated, string& attributeName);
    ~GetAttributeExp() override{
        delete(object);
    }
    Error * checkReference(Scope* scope) override{ return nullptr;}
    void acceptDereferenceCheck(DereferenceChecker *checker) override;

};

class InvokeExp:public Exp{
public:
    string functionName;
    Args *args;
    explicit InvokeExp(AttrNode* invoker);
    InvokeExp(AttrNode* invoker, AttrNode* args);
    ~InvokeExp() override{
        delete(args);
    }
    Error * checkReference(Scope* scope) override;
    void acceptDereferenceCheck(DereferenceChecker* checker) override ;
};


#endif //SPL_EXPRESSION_H
