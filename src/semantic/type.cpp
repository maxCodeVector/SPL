//
// Created by hya on 11/19/19.
//
#include "type.h"
#include "../astnode/statement.h"
#include "scope.h"

bool hasLoop(set<string> &mark, Struct *type) {
    for (Variable *var :type->getMemberList()) {
        VariableType *member_type = var->getType();
        if (member_type->getElementType() != STRUCT_TYPE)
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
            errorHandler.recordError(new Error{
                    itor->second->getLocation(), ErrorType::RECURSIVE_DEFINE,
                    "Recursive Definition for:" + type_name});
        }
        itor++;
    }

}

void TypeTable::declareVariableType(VariableType *variableType, ErrorHandler &err) {
    if (variableType->getElementType() == STRUCT_TYPE) {
        string &name = ((Struct *) variableType)->getTypeName();
        auto Itor = declaredTypes.find(name);
        if (Itor == declaredTypes.end() || !Itor->second->isComplete()) {
            this->declaredTypes.erase(name);
            this->declaredTypes.insert(pair<string, VariableType *>(name, variableType));
        } else {
            Error *error = new Error{variableType->getLocation(), REDEFINED_STRUCT,
                                     "redefine Struct multi times:" + name};
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


DereferenceChecker::DereferenceChecker(ErrorHandler &errorHandle, TypeTable *type_table) :
        Visitor(errorHandle, type_table) {
}


void DereferenceChecker::resolve(AST &ast) {
    for (Function *function: ast.getFunctions()) {
        resolve(*function->getBody());
    }
}

void DereferenceChecker::resolve(Body &body) {
    body.acceptDereferenceCheck(this);
}

TypeChecker::TypeChecker(ErrorHandler &errorHandle, TypeTable *type_table) : Visitor(errorHandle, type_table) {
    toplevelScope = nullptr;
}

void TypeChecker::resolve(AST &ast) {
    toplevelScope = ast.getScope();
    for (Function *function: ast.getFunctions()) {
        if (function->flag != BUILD_NODE)
            checkFunction(function);
        checkReturnType(function->getReturnType());
    }
}

void TypeChecker::checkReturnType(VariableType *returnType) {
    if (returnType->getElementType() != INT_TYPE
        && returnType->getElementType() != CHAR_TYPE
        && returnType->getElementType() != FLOAT_TYPE) {
        error(new Error{returnType->getLocation(), OTHER_ERROR, "only support return int/char/float"});
    }
}

void TypeChecker::checkFunction(Function *function) {
    function->getBody()->checkMembersType(this, function);
}


