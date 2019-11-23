//
// Created by hya on 11/20/19.
//

#include "statement.h"
#include "expression.h"
#include "../resolver.h"

Statement::Statement(AttrNode *exp) {
    this->exp = (Exp *) exp->baseNode;
    setLocation(exp->lineNo, 0);
}

void Statement::checkReference(LocalResolver *resolver, Scope *scope) {
    resolver->error(exp->checkReference(scope));
}


void Statement::acceptDereferenceCheck(DereferenceChecker *checker) {
    if (exp) {
        exp->acceptDereferenceCheck(checker);
    }
}

void Statement::checkMembersType(TypeChecker *checker, DefinedFunction *function) {
    checker->error(exp->checkType(checker->getTopLevelScope()));

}

Statement::~Statement() {
    delete (exp);
}


Body::Body(AttrNode *defList, AttrNode *stmtList) {
    exp = nullptr;
    this->flag = BODY;
    DefinedVariable *variable = (DefinedVariable *) defList->baseNode;
    while (variable != nullptr) {
        this->vars.push_back(variable);
        variable = (DefinedVariable *) variable->next;
    }
    Statement *statement = (Statement *) stmtList->baseNode;
    while (statement != nullptr) {
        this->statements.push_back(statement);
        statement = (Statement *) statement->next;
    }
    setLocation(defList->lineNo, 0);
}


void Body::checkReference(LocalResolver *resolver, Scope *scope) {
    list<DefinedVariable *> emptyParaList;
    resolver->pushScope(emptyParaList);
    resolver->resolve(*this);
    resolver->popScope();
}

void Body::acceptDereferenceCheck(DereferenceChecker *checker) {
    for (DefinedVariable *var: vars) {
        Exp *value = var->getValue();
        if (value != nullptr) {
            value->acceptDereferenceCheck(checker);
        }
    }
    for (Statement *statement:statements) {
        statement->acceptDereferenceCheck(checker);
    }
}

void Body::checkMembersType(TypeChecker *checker, DefinedFunction *function) {
    for (DefinedVariable *var: this->vars) {
        Exp *value = var->getValue();
        if (value) {
            Error *err = value->checkType(checker->getTopLevelScope());
            if (err) {
                checker->error(err);
                continue;
            }
            bool equal = CheckEqualVariableAndExp(var, value);
            if (!equal) {
                checker->error(new Error{value->getLocation(),ASSIGN_DIFF_TYPE,
                                         "initial variable using a different type value" + var->getName()});
                continue;
            }
        }
    }
    for (Statement *statement: this->statements) {
        statement->checkMembersType(checker, function);
    }
}

void IfStatement::checkReference(LocalResolver *resolver, Scope *scope) {
    Statement::checkReference(resolver, scope);
    if (ifBody)
        ifBody->checkReference(resolver, scope);
    if (elseBody)
        elseBody->checkReference(resolver, scope);
}

void IfStatement::acceptDereferenceCheck(DereferenceChecker *checker) {
    Statement::acceptDereferenceCheck(checker);
    if (ifBody) {
        ifBody->acceptDereferenceCheck(checker);
    }
    if (elseBody) {
        elseBody->acceptDereferenceCheck(checker);
    }
}

void IfStatement::checkMembersType(TypeChecker *checker, DefinedFunction *function) {
    Statement::checkMembersType(checker, function);
    if(exp->getType()->getType()!=BOOL_TYPE){
        checker->error(new Error{exp->getLocation(), OTHER_ERROR, "condition of if should be a bool type"});
    }
    if (ifBody)
        ifBody->checkMembersType(checker, function);
    if (elseBody)
        elseBody->checkMembersType(checker, function);
}


void WhileStatement::checkReference(LocalResolver *resolver, Scope *scope) {
    Statement::checkReference(resolver, scope);
    if (loop)
        loop->checkReference(resolver, scope);
}


void WhileStatement::acceptDereferenceCheck(DereferenceChecker *checker) {
    Statement::acceptDereferenceCheck(checker);
    if (loop)
        loop->acceptDereferenceCheck(checker);
}


void WhileStatement::checkMembersType(TypeChecker *checker, DefinedFunction *function) {
    Statement::checkMembersType(checker, function);
    if(exp->getType()->getType()!=BOOL_TYPE){
        checker->error(new Error{exp->getLocation(), OTHER_ERROR, "condition of while should be a bool type"});
    }
    if (loop)
        loop->checkMembersType(checker, function);
}

void ReturnStatement::checkMembersType(TypeChecker *checker, DefinedFunction *function) {
    Statement::checkMembersType(checker, function);
    if (this->exp->getType()->getType() != function->getReturnType()->getType()) {
        Error *err = new Error{getLocation(), MIS_RETURN_TYPE,
                               "return type is not the same as declared：" + function->getName()};
        checker->error(err);
    }
}
