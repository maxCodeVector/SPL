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


class DereferenceChecker : public Visitor{
    void checkStatement(Statement* statement);
public:
    void resolve(AST& ast) override;
    void resolve(Body& body) override;
    explicit DereferenceChecker(ErrorHandler& errorHandle, TypeTable* type_table);
};


class TypeChecker: public Visitor{
public:
    void resolve(AST& ast) override;
    void resolve(Body& body) override;
    explicit TypeChecker(ErrorHandler& errorHandle, TypeTable* type_table);

    void checkReturnType(VariableType *type);
};


#endif //SPL_TYPE_H
