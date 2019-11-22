//
// Created by hya on 11/19/19.
//

#ifndef SPL_TYPE_H
#define SPL_TYPE_H

#include <set>
#include "ast.h"
#include "error.h"


class TypeTable{
    map<string, VariableType*> declaredTypes;
    bool hasLoop(set<string >& mark, Struct* type);

public:
    void checkRecursiveDefinition(ErrorHandler& errorHandler);

    void declareVariableType(VariableType *variableType, ErrorHandler &err);

    VariableType * queryType(string &name);

};


class DereferenceChecker : Visitor{
public:
    void resolve(AST& ast) override;
    void resolve(Body& body) override;
    explicit DereferenceChecker(ErrorHandler& errorHandle);
};


class TypeChecker: Visitor{
public:
    void resolve(AST& ast) override;
    void resolve(Body& body) override;
    explicit TypeChecker(ErrorHandler& errorHandle);

};


#endif //SPL_TYPE_H
