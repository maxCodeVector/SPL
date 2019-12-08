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
        function->setIr(complileFunctionBody(function));
    }
    if (this->errorHandler.errorOccured()){
        return nullptr;
    }
    return ast.getIR();
}

IRStatement* IRGenerator::complileFunctionBody(Function *f) {
    if(f->flag==BUILD_NODE){
        return nullptr;
    }
    IRStatement* irStatement = new IRStatement;
    irStatement->addInstruction(IROperator::IR_FUNCTION, f->getName());
    free_all(this->scopeStack);
    this->jumpMap.clear();
    transformStmt(f->getBody());
    checkJumpLinks(jumpMap);
    return irStatement;
}

void IRGenerator::checkJumpLinks(map<string, JumpEntry> &maps) {

}

void IRGenerator::visit(Exp *expNode) {

}

void IRGenerator::visit(Statement *statementNode) {

}
