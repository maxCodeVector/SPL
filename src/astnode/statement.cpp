//
// Created by hya on 11/20/19.
//

#include "statement.h"
#include "expression.h"
#include "../semantic/resolver.h"
#include "../ir/irnode.h"

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

void Statement::checkMembersType(TypeChecker *checker, Function *function) {
    checker->error(exp->checkType(checker->getTopLevelScope()));

}

Statement::~Statement() {
    delete (exp);
}

void Statement::accept(IRVisitor *visitor) {
    exp->accept(visitor);
}


Body::Body(AttrNode *defList, AttrNode *stmtList) {
    exp = nullptr;
    this->flag = BODY;
    Variable *variable = (Variable *) defList->baseNode;
    while (variable != nullptr) {
        this->vars.push_back(variable);
        variable = (Variable *) variable->next;
    }
    Statement *statement = (Statement *) stmtList->baseNode;
    while (statement != nullptr) {
        this->statements.push_back(statement);
        statement = (Statement *) statement->next;
    }
    setLocation(defList->lineNo, 0);
}


void Body::checkReference(LocalResolver *resolver, Scope *scope) {
    list<Variable *> emptyParaList;
    resolver->pushScope(emptyParaList);
    resolver->resolve(*this);
    resolver->popScope();
}

void Body::acceptDereferenceCheck(DereferenceChecker *checker) {
    for (Variable *var: vars) {
        Exp *value = var->getInitializer();
        if (value != nullptr) {
            value->acceptDereferenceCheck(checker);
        }
    }
    for (Statement *statement:statements) {
        statement->acceptDereferenceCheck(checker);
    }
}

void Body::checkMembersType(TypeChecker *checker, Function *function) {
    for (Variable *var: this->vars) {
        Exp *value = var->getInitializer();
        if (value) {
            Error *err = value->checkType(checker->getTopLevelScope());
            if (err) {
                checker->error(err);
                continue;
            }
            bool equal = CheckEqualVariableAndExp(var, value);
            if (!equal) {
                checker->error(new Error{value->getLocation(), ASSIGN_DIFF_TYPE,
                                         "initial variable using a different elementType value" + var->getName()});
                continue;
            }
        }
    }
    for (Statement *statement: this->statements) {
        statement->checkMembersType(checker, function);
    }
}

void Body::accept(IRVisitor *visitor) {
    for(Variable* variable: this->vars){

    }
    for(Statement* statement: statements){
        statement->accept(visitor);
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

void IfStatement::checkMembersType(TypeChecker *checker, Function *function) {
    Statement::checkMembersType(checker, function);
    if (exp->getType()->getElementType() != BOOL_TYPE) {
        checker->error(new Error{exp->getLocation(), OTHER_ERROR, "condition of if should be a bool elementType"});
    }
    if (ifBody)
        ifBody->checkMembersType(checker, function);
    if (elseBody)
        elseBody->checkMembersType(checker, function);
}

void IfStatement::accept(IRVisitor *visitor) {
    exp->accept(visitor);
    ifBody->accept(visitor);
    if(elseBody)
        elseBody->accept(visitor);
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


void WhileStatement::checkMembersType(TypeChecker *checker, Function *function) {
    Statement::checkMembersType(checker, function);
    if (exp->getType()->getElementType() != BOOL_TYPE) {
        checker->error(new Error{exp->getLocation(), OTHER_ERROR, "condition of while should be a bool elementType"});
    }
    if (loop)
        loop->checkMembersType(checker, function);
}

void WhileStatement::accept(IRVisitor *visitor) {
    exp->accept(visitor);
    loop->accept(visitor);
}

void ReturnStatement::checkMembersType(TypeChecker *checker, Function *function) {
    Statement::checkMembersType(checker, function);
    // no repeat report expression error
    if (this->exp->getType()->getElementType() != INFER_TYPE) {
        if (exp->isArray()
            || (this->exp->getType()->getElementType() != function->getReturnType()->getElementType()
            && function->getReturnType()->getElementType() != FLOAT_TYPE
            && this->exp->getType()->getElementType() != INT_TYPE)) {
            Error *err = new Error{getLocation(), MIS_RETURN_TYPE,
                                   "return elementType is not the same as declared：" + function->getName()};
            checker->error(err);
        }
    }
}

void ReturnStatement::accept(IRVisitor *visitor) {
    exp->accept(visitor);
    visitor->visit(this);
}
