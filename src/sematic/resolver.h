//
// Created by hya on 11/22/19.
//

#ifndef SPL_RESOLVER_H
#define SPL_RESOLVER_H

#include "scope.h"
#include "../ast.h"

class LocalResolver:public Visitor{
private:
    list<Scope*> scopeStack;
    ConstantTable constantTable;

    Scope* currentScope(){
        return scopeStack.back();
    }

public:
    explicit LocalResolver(ErrorHandler &h, TypeTable* type_table): Visitor(h, type_table){
    }
    ~LocalResolver(){
        while (!scopeStack.empty()) {
            scopeStack.pop_front();
        }
    }
    void resolve(Body& body);
    void resolve(AST& ast) override;
    void resolveDeclaredType(list<VariableType*> & declared);
    void resolveFunctions(list<Function*>& funcs);
    void pushScope(list<Variable*>& vars);
    Scope* popScope();

};

class TypeResolver : Visitor{
private:
    void resolveFunctions(list<Function*>& funs);
    void resolve(Body& body);
    void resolveStatement(Statement* statement);
public:
    explicit TypeResolver(ErrorHandler& errorHandle, TypeTable* type_table);
    void resolve(AST& ast) override;
};

#endif //SPL_RESOLVER_H
