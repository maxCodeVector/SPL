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
    list<string> breakStack;
    list<string> continueStack;
    AST *currAst;
    bool optimized;

    map<string, string> symbolAddrTable;
    TempNameGenerator *varVariable;
    TempNameGenerator *tempVariable;
    TempNameGenerator *labelGenerator;
    TempNameGenerator *pointer;
    IRStatement *currIrStatement;
    map<string, JumpEntry *> jumpMap;
    map<Operator, IROperator> arithmeticMap;
    map<Operator, IROperator> compareOppositeMap;

    void getValueInBinaryExp(Exp *left, Exp *right, string *leftSymbol, string *rightSymbol);

    void addInstToJumpEntry(const string &labelName, IRInst *inst);

    void transformStmt(Statement *statement);

    void checkJumpLinks(IR *ir);

    IRStatement *complileFunctionBody(Function *f);

    void initCurrScopeSymbol(LocalScope *localScope);

    void translateConditionExp(const Exp *expNode, const string &label_t, const string &label_f);

    void removeGotoInstFromJumpMap(IRInst *gotoInst);

public:
    explicit IRGenerator(bool optimized);

    IR *generate(AST &ast);

    string getAddress(string &id) override;

    void handleBreakAndContinue(Operator bc, Location *location) override;

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
