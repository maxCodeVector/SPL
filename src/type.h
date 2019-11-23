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

public:
    void checkRecursiveDefinition(ErrorHandler& errorHandler);

    void declareVariableType(VariableType *variableType, ErrorHandler &err);

    VariableType * queryType(string &name);

};


/**
 * to check if there are expression assign a value to non left value type variable or get a member to a non-struct
 */
class DereferenceChecker : public Visitor{
    void checkStatement(Statement* statement);
    void resolve(Body& body);

public:
    void resolve(AST& ast) override;
    explicit DereferenceChecker(ErrorHandler& errorHandle, TypeTable* type_table);
};


class TypeChecker: public Visitor{
    ToplevelScope* toplevelScope;
    void checkReturnType(VariableType* type);
    void checkFunction(DefinedFunction* function);

public:
    void resolve(AST& ast) override;
    explicit TypeChecker(ErrorHandler& errorHandle, TypeTable* type_table);
    ToplevelScope* getTopLevelScope(){
        return toplevelScope;
    }
};


#endif //SPL_TYPE_H
