//
// Created by hya on 11/20/19.
//

#ifndef SPL_EXPRESSION_H
#define SPL_EXPRESSION_H

#include "extnode.h"

class ToplevelScope;
class DereferenceChecker;
class IRExpr;
class IRVisitor;
class Exp:public BaseNode{
protected:
    string symbol;
public:
    const string &getSymbol() const;

    void setSymbol(const string &symbol);

protected:
    enum Operator operatorType;
public:
    Operator getOperatorType() const;

protected:
    VariableType* type;
    string value;//expression value if it is ID or int:float:char
    Entity* referenceVar;
    int dimension;// used for temp_array_size, means which dimension current is in
public:
    Exp(AttrNode* terminal, DataType dataType);
    Exp(DataType dataType);
    virtual ~Exp(){
//        delete(elementType);
    }
    string& getValue(){
        return value;
    }
    void setType(VariableType* type){
        this->type = type;
    }
    int indexOneDimension(int curr){
        this->dimension = curr + 1;
        return dimension;
    }
    int getCurrentDimension(){
        return dimension;
    }
    /**
     * if this exp type is a reference type, then it will try to return its actual type
     * if its referenceVar is not NULL
     * @return
     */
    VariableType* getType();
    Variable* getReferenceValue(){
        return (Variable*)referenceVar;
    }
    void setReferenceVar(Entity* entity)
    {
        referenceVar=entity;
    }
    Error *checkType(ToplevelScope *topLevel) { return inferType(topLevel);}
    bool isLeftValue();
    bool isArray();
    bool isNumber();
    virtual Error * checkReference(Scope* scope);
    virtual void acceptDereferenceCheck(DereferenceChecker* checker){}
    virtual Error *inferType(ToplevelScope *topLevel);
    virtual void accept(IRVisitor* visitor);
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
    void acceptDereferenceCheck(DereferenceChecker *checker) override;
    Error* inferType(ToplevelScope* toplevelScope) override;
    void accept(IRVisitor* visitor) override;

};

class UnaryExp: public Exp{
public:
    Exp* operand;
    UnaryExp(AttrNode* operated, Operator operatorType);
    ~UnaryExp() override{
        delete(operand);
    }
    Error * checkReference(Scope* scope) override;
    Error* inferType(ToplevelScope* toplevelScope) override;

};

class GetAttributeExp: public Exp{
    string attrName;
public:
    Exp* object;
    GetAttributeExp(AttrNode* operated, string& attributeName);
    ~GetAttributeExp() override{
        delete(object);
    }
    Error * checkReference(Scope* scope) override;
    void acceptDereferenceCheck(DereferenceChecker *checker) override;
    Error* inferType(ToplevelScope* toplevelScope) override;

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
    void acceptDereferenceCheck(DereferenceChecker* checker) override;
    Error* inferType(ToplevelScope* toplevelScope) override;

};

bool checkTypeEqualOfExp(Exp* exp1, Exp* exp2);
bool CheckEqualVariableAndExp(Variable* var, Exp* exp);


#endif //SPL_EXPRESSION_H
