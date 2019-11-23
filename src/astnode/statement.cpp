//
// Created by hya on 11/20/19.
//

#include "statement.h"
#include "expression.h"
#include "../resolver.h"

Body::Body(AttrNode *defList, AttrNode *stmtList) {
    exp = nullptr;
    this->flag = BODY;
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

bool checkTypeEqual(VariableType *src, VariableType *target) {
    if(src->getType()!=target->getType()){
        return false;
    }
    if(src->getType()==STRUCT_TYPE){

    }
    return true;
}

void Body::checkMembersType(TypeChecker *checker) {
    for (DefinedVariable *var: this->vars) {
        Exp *value = var->getValue();
        if (value) {
            Error *err = value->checkType(nullptr);
            if (err) {
                checker->error(err);
                continue;
            }
            bool equal = checkTypeEqual(var->getType(), value->getType());
            if(!equal){
                checker->error(new Error{value->getLocation(), "the two value type not the same"});
                continue;
            }
        }
    }
    for(Statement* statement: this->statements){
        statement->checkMembersType(checker);
    }
}

void Body::acceptDereferenceCheck(DereferenceChecker *checker) {
    for(DefinedVariable* var: vars){
        Exp* value = var->getValue();
        if(value!= nullptr){
            value->acceptDereferenceCheck(checker);
        }
    }
    for (Statement *statement:statements) {
        statement->acceptDereferenceCheck(checker);
    }
}

void Body::checkReference(LocalResolver *resolver, Scope *scope) {
    for(DefinedVariable* var: this->vars){
        if(var->getValue())
            resolver->error(var->getValue()->checkReference(scope));
    }
    for(Statement* statement: this->statements){
        statement->checkReference(resolver, scope);
    }
}


Statement::Statement(AttrNode *exp) {
    this->exp = (Exp*)exp->baseNode;
}

void Statement::checkMembersType(TypeChecker* checker) {
    checker->error(exp->checkType(checker->getTopLevelScope()));

}

Statement::~Statement() {
    delete(exp);
}

void Statement::acceptDereferenceCheck(DereferenceChecker *checker) {
    if(exp){
        exp->acceptDereferenceCheck(checker);
    }
}

void Statement::checkReference(LocalResolver *resolver, Scope *scope) {
    resolver->error(exp->checkReference(scope));
}


void IfStatement::checkMembersType(TypeChecker *checker) {
    Statement::checkMembersType(checker);
    if(ifBody)
        ifBody->checkMembersType(checker);
    if(elseBody)
        elseBody->checkMembersType(checker);
}

void IfStatement::acceptDereferenceCheck(DereferenceChecker *checker) {
    Statement::acceptDereferenceCheck(checker);
    if(ifBody){
        ifBody->acceptDereferenceCheck(checker);
    }
    if(elseBody){
        elseBody->acceptDereferenceCheck(checker);
    }
}

void IfStatement::checkReference(LocalResolver *resolver, Scope *scope) {
    Statement::checkReference(resolver, scope);
    if(ifBody)
        ifBody->checkReference(resolver,scope);
    if(elseBody)
        elseBody->checkReference(resolver, scope);
}

void WhileStatement::acceptDereferenceCheck(DereferenceChecker *checker) {
    Statement::acceptDereferenceCheck(checker);
    if(loop)
        loop->acceptDereferenceCheck(checker);
}

void WhileStatement::checkReference(LocalResolver *resolver, Scope *scope) {
    Statement::checkReference(resolver, scope);
    if(loop)
        loop->checkReference(resolver, scope);
}

void WhileStatement::checkMembersType(TypeChecker *checker) {
    Statement::checkMembersType(checker);
    if(loop)
        loop->checkMembersType(checker);
}
