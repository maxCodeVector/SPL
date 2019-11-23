//
// Created by hya on 11/19/19.
//
#include "type.h"
#include "astnode/statement.h"
#include "scope.h"

bool TypeTable::hasLoop(set<string> &mark, Struct *type) {
    for (DefinedVariable *var :type->getMemberList()) {
        VariableType *member_type = var->getType();
        if (member_type->getType() != STRUCT_TYPE)
            continue;
        string &type_name = member_type->getTypeName();
        auto item = mark.find(type_name);
        if (item != mark.end()) {
            return true;
        }
        mark.insert(type_name);
        bool loop = hasLoop(mark, (Struct *) member_type->getActualType());
        if (loop)
            return true;
    }
    return false;
}

void TypeTable::checkRecursiveDefinition(ErrorHandler &errorHandler) {
    auto itor = this->declaredTypes.begin();
    while (itor != declaredTypes.end()) {
        set<string> mark;
        string &type_name = itor->second->getTypeName();
        mark.insert(type_name);
        bool loop = hasLoop(mark, (Struct *) itor->second);
        if (loop) {
            errorHandler.recordError(new Error{itor->second->getLocation(),
                                               "Recursive Definition for:" + type_name});
//            return;
        }
        itor++;
    }

}

void TypeTable::declareVariableType(VariableType *variableType, ErrorHandler &err) {
    if (variableType->getType() == STRUCT_TYPE) {
        string &name = ((Struct *) variableType)->getTypeName();
        auto Itor = declaredTypes.find(name);
        if (Itor == declaredTypes.end() || !Itor->second->isComplete()) {
            this->declaredTypes.erase(name);
            this->declaredTypes.insert(pair<string, VariableType *>(name, variableType));
        } else {
            Error *error = new Error{variableType->getLocation(), "redefine variable type multi times:" + name};
            err.recordError(error);
        }
    }
}

VariableType *TypeTable::queryType(string &name) {
    auto Itor = declaredTypes.find(name);
    if (Itor == declaredTypes.end()) {
        return nullptr;
    }
    return Itor->second;
}


void DereferenceChecker::resolve(AST &ast) {
    for (DefinedFunction *function: ast.defineFunctions()) {
        resolve(*function->getBody());
    }
}



void DereferenceChecker::resolve(Body &body) {
    body.acceptDereferenceCheck(this);
}



DereferenceChecker::DereferenceChecker(ErrorHandler &errorHandle, TypeTable *type_table) :
        Visitor(errorHandle, type_table) {

}

void DereferenceChecker::checkStatement(Statement *statement) {
    Exp* exp = statement->getExpression();
    if(exp){
        exp->acceptDereferenceCheck(this);
    }
    if(statement->flag==BODY){
        resolve(*(Body*)statement);
    }
}


void TypeChecker::resolve(AST &ast) {
    for (DefinedFunction *function: ast.defineFunctions()) {
        checkReturnType(function->getReturnType());
        resolve(*function->getBody());
    }
}

void TypeChecker::resolve(Body &body) {
    body.checkMembersType(this->errorHandler);
}


TypeChecker::TypeChecker(ErrorHandler &errorHandle, TypeTable *type_table) : Visitor(errorHandle, type_table) {

}

void TypeChecker::checkReturnType(VariableType *returnType) {
    if(returnType->getType()!=INT_TYPE
    && returnType->getType()!=CHAR_TYPE
    && returnType->getType()!=FLOAT_TYPE){
        error(new Error{returnType->getLocation(), "only support return int/char/float"});
    }
}


