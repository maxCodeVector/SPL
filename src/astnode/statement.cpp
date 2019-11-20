//
// Created by hya on 11/20/19.
//

#include "statement.h"
#include "expression.h"

Body::Body(AttrNode *defList, AttrNode *stmtList) {
    exp = nullptr;
    DefinedVariable* variable = (DefinedVariable*)defList->baseNode;
    while (variable!= nullptr){
        this->vars.push_back(variable);
        variable = (DefinedVariable*)variable->next;
    }
    Statement* statement = (Statement*)stmtList->baseNode;
    while (statement!= nullptr){
        this->statements.push_back(statement);
        statement = (Statement*)statement->next;
    }
}

void Body::checkMembersType(ErrorHandler& handler) {
    for(Statement* statement: this->statements){
        statement->checkMembersType(handler);
    }
}


Statement::Statement(AttrNode *exp) {
    this->exp = (Exp*)exp->baseNode;
}

void Statement::checkMembersType(ErrorHandler& handler) {
    Error* error =  exp->checkType();
    if(error!= nullptr)
        handler.recordError(error);
}

Statement::~Statement() {
    delete(exp);
}

void IfStatement::checkMembersType(ErrorHandler &handler) {
    Statement::checkMembersType(handler);
    if(ifBody)
        ifBody->checkMembersType(handler);
    if(elseBody)
        elseBody->checkMembersType(handler);
}
