//
// Created by hya on 12/8/19.
//

#include "IRGenerator.h"


void IRGenerator::transformStmt(Statement *statement) {
    statement->accept(this);
}

IRExpr *IRGenerator::transformExpr(Exp *exp) {
    exprNestLevel++;
    IRExpr *e = exp->accept(this);
    exprNestLevel--;
    return e;
}

bool IRGenerator::isStatement() {
    return exprNestLevel == 0;
}

IR *IRGenerator::generate(AST &ast) {
    for (Variable *variable: ast.getDefinedVars()) {
        if (variable->hasInitializer()) {
            variable->setIR(transformExpr(variable->getInitializer()));
        }
    }
    for (Function *function:ast.getFunctions()) {
        function->setIR(complileFunctionBody(function));
    }
    if (this->errorHandler.errorOccured()){
        return nullptr;
    }
    return ast.getIR();
}

list<IRStatement *> *IRGenerator::complileFunctionBody(Function *f) {
    auto *list = new std::list<IRStatement *>();
    if(f->flag==BUILD_NODE){
        return list;
    }

    free_all(this->scopeStack);
    this->jumpMap.clear();
    transformStmt(f->getBody());
    checkJumpLinks(jumpMap);
    return list;
}

void IRGenerator::checkJumpLinks(map<string, JumpEntry> &maps) {

}

void IRGenerator::visit(Exp *expNode) {

}

void IRGenerator::visit(Statement *statementNode) {

}
