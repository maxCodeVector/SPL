//
// Created by hya on 11/20/19.
//

#ifndef SPL_STATEMENT_H
#define SPL_STATEMENT_H
#include "../error.h"
#include "extnode.h"

class Statement:public BaseNode{
protected:
    Exp *exp;
public:
    explicit Statement(AttrNode* exp);
    Statement(){}
    virtual ~Statement();
    Exp* getExpression(){
        return exp;
    }
    virtual void checkMembersType(ErrorHandler& handler);
};

class BreakStatement{
    virtual bool checkBreak() = 0;
};

class ContinueStatement{
    virtual bool checkContinue() = 0;
};


class Body : public Statement, BreakStatement, ContinueStatement{
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

    void checkMembersType(ErrorHandler& handler) override;

};

class IfStatement:public Statement{
public:
    Statement* ifBody;
    Statement* elseBody;
    IfStatement(AttrNode* exp, AttrNode* ifNode, AttrNode* elseNode):Statement(exp){
        ifBody  = (Statement*)ifNode->baseNode;
        elseBody = (Statement*)elseNode->baseNode;
    }
    IfStatement(AttrNode* exp, AttrNode* ifNode):Statement(exp){
        ifBody  = (Statement*)ifNode->baseNode;
    }
    ~IfStatement(){
        delete(ifBody);
        delete(elseBody);
    }
    void checkMembersType(ErrorHandler& handler) override;

};

class WhileStatement:public Statement{
public:
    Statement* loop;
    WhileStatement(AttrNode* exp, AttrNode* loopNode):Statement(exp){
        loop = (Statement*)loopNode->baseNode;
    }
    ~WhileStatement(){
        delete(loop);
    }
};

class ReturnStatement:public Statement{
public:
    explicit ReturnStatement(AttrNode* exp):Statement(exp){}
};



#endif //SPL_STATEMENT_H
