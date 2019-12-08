//
// Created by hya on 12/8/19.
//

#ifndef SPL_IRGENERATOR_H
#define SPL_IRGENERATOR_H

#include "../ast.h"
#include "irnode.h"
#include "../semantic/scope.h"

class IRGenerator : public IRVisitor {
    list<LocalScope *> scopeStack;
    map<string, JumpEntry> jumpMap;
    int exprNestLevel = 0;

    void transformStmt(Statement *statement);

    IRExpr *transformExpr(Exp *exp);

    bool isStatement();

    void checkJumpLinks(map<string, JumpEntry> &maps);

    IRStatement* complileFunctionBody(Function *f);

public:
    IR *generate(AST &ast);

    void visit(Exp *expNode) override;

    void visit(Statement *statementNode) override;

};


#endif //SPL_IRGENERATOR_H
