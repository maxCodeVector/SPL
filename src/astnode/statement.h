//
// Created by hya on 11/20/19.
//

#ifndef SPL_STATEMENT_H
#define SPL_STATEMENT_H

#include "../error.h"
#include "extnode.h"

class TypeChecker;

class DereferenceChecker;

class LocalResolver;

class IRVisitor;

class Statement : public BaseNode {
protected:
    Exp *exp;
public:
    explicit Statement(AttrNode *exp);

    Statement() {}

    virtual ~Statement();

    Exp *getExpression() {
        return exp;
    }

    virtual void checkMembersType(TypeChecker *checker, Function *function);

    virtual void acceptDereferenceCheck(DereferenceChecker *checker);

    virtual void accept(IRVisitor *visitor);

    virtual void checkReference(LocalResolver *resolver, Scope *scope);
};

class BreakStatement {
    virtual bool checkBreak() = 0;
};

class ContinueStatement {
    virtual bool checkContinue() = 0;
};


class Body : public Statement, BreakStatement, ContinueStatement {
public:
    list<Variable *> vars;
    list<Statement *> statements;

    Body(AttrNode *defList, AttrNode *stmtList);

    ~Body() {
        free_all(vars);
        free_all(statements);
    }

    bool checkContinue() override { return true; };

    bool checkBreak() override { return true; };

    void checkMembersType(TypeChecker *checker, Function *function) override;

    void acceptDereferenceCheck(DereferenceChecker *checker) override;

    void checkReference(LocalResolver *resolver, Scope *scope) override;

    void accept(IRVisitor *visitor) override;

};

class IfStatement : public Statement {
public:
    Statement *ifBody;
    Statement *elseBody;

    IfStatement(AttrNode *exp, AttrNode *ifNode, AttrNode *elseNode) : Statement(exp) {
        ifBody = (Statement *) ifNode->baseNode;
        elseBody = (Statement *) elseNode->baseNode;
    }

    IfStatement(AttrNode *exp, AttrNode *ifNode) : Statement(exp) {
        ifBody = (Statement *) ifNode->baseNode;
    }

    ~IfStatement() {
        delete (ifBody);
        delete (elseBody);
    }

    void checkMembersType(TypeChecker *checker, Function *function) override;

    void acceptDereferenceCheck(DereferenceChecker *checker) override;

    void checkReference(LocalResolver *resolver, Scope *scope) override;

    void accept(IRVisitor *visitor) override;

};

class WhileStatement : public Statement {
public:
    Statement *loop;

    WhileStatement(AttrNode *exp, AttrNode *loopNode) : Statement(exp) {
        loop = (Statement *) loopNode->baseNode;
    }

    ~WhileStatement() {
        delete (loop);
    }

    void acceptDereferenceCheck(DereferenceChecker *checker) override;

    void checkReference(LocalResolver *resolver, Scope *scope) override;

    void checkMembersType(TypeChecker *checker, Function *function) override;

    void accept(IRVisitor *visitor) override;
};

class ReturnStatement : public Statement {
public:
    explicit ReturnStatement(AttrNode *exp) : Statement(exp) {}

    void checkMembersType(TypeChecker *checker, Function *function) override;

    void accept(IRVisitor *visitor) override;
};


#endif //SPL_STATEMENT_H
