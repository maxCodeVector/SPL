//
// Created by hya on 12/8/19.
//

#ifndef SPL_IRGENERATOR_H
#define SPL_IRGENERATOR_H

#include "../ast.h"
#include "irnode.h"
#include "../semantic/scope.h"

class TempNameGenerator {
    string prefix;
    int *allocated;
    int curr_max_id_num;
public:
    explicit TempNameGenerator(const string &prefix, int size);

    int allocate();

    void release(int numberId);

    void releaseAll();

    ~TempNameGenerator();

    string generateName(int numId);

};


class IRGenerator : public IRVisitor {
    list<LocalScope *> scopeStack;
    TempNameGenerator *tempVariable;
    TempNameGenerator *label;
    IRStatement *currIrStatement;
    map<string, JumpEntry> jumpMap;
    map<Operator, IROperator> operatorMap;

    int exprNestLevel = 0;

    void transformStmt(Statement *statement);

    IRExpr *transformExpr(Exp *exp);

    bool isStatement();

    void checkJumpLinks(map<string, JumpEntry> &maps);

    IRStatement *complileFunctionBody(Function *f);

public:
    IRGenerator();

    IR *generate(AST &ast);

    void visit(BinaryExp *expNode) override;

    void visit(ReturnStatement *statementNode) override;

};


#endif //SPL_IRGENERATOR_H
