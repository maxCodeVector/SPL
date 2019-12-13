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

    void releaseAll(bool);

    ~TempNameGenerator();

    string generateName(int numId);

};


class IRGenerator : public IRVisitor {
    list<string> labelStack;
    list<string> breakStack;
    list<string> continueStack;
    AST *currAst;

    map<string, string> symbolAddrTable;
    TempNameGenerator *varVariable;
    TempNameGenerator *tempVariable;
    TempNameGenerator *label;
    TempNameGenerator *pointer;
    IRStatement *currIrStatement;
    map<string, JumpEntry *> jumpMap;
    map<Operator, IROperator> arithmeticMap;
    map<Operator, IROperator> compareOppositeMap;
    int exprNestLevel = 0;

    void getValueInBinaryExp(Exp *left, Exp *right, string *leftSymbol, string *rightSymbol);

    void addInstToJumpEntry(string &labelName, IRInst *inst);

    void transformStmt(Statement *statement);

    IRExpr *transformExpr(Exp *exp);

    bool isStatement();

    void checkJumpLinks(IR *ir);

    IRStatement *complileFunctionBody(Function *f);

    void initCurrScopeSymbol(LocalScope *localScope);

public:
    IRGenerator();

    IR *generate(AST &ast);

    string getAddress(string &id) override;

    void visit(BinaryExp *expNode) override;

    void visit(UnaryExp *expNode) override;

    void visit(InvokeExp *expNode) override;

    void visit(GetAttributeExp *expNode) override;

    void visit(Variable *variable) override;

    void visit(ReturnStatement *statementNode) override;

    void visit(IfStatement *statementNode) override;

    void visit(WhileStatement *statementNode) override;

};


#endif //SPL_IRGENERATOR_H
