//
// Created by hya on 11/22/19.
//

#ifndef SPL_RESOLVER_H
#define SPL_RESOLVER_H

#include "scope.h"
#include "ast.h"

class LocalResolver:public Visitor{
private:
    list<Scope*> scopeStack;
    ConstantTable constantTable;

    Scope* currentScope(){
        return scopeStack.back();
    }
    void resolve(Body& body);

public:
    explicit LocalResolver(ErrorHandler &h, TypeTable* type_table): Visitor(h, type_table){
    }
    ~LocalResolver(){
        while (!scopeStack.empty()) {
            scopeStack.pop_front();
        }
    }


    void resolveDeclaredType(list<VariableType*> & declared);

    void resolve(AST& ast) override;

    void resolve(Statement& statement);

    void resolveFunctions(list<DefinedFunction*>& funcs);


    void pushScope(list<DefinedVariable*>& vars);
    Scope* popScope(){
        Scope* scope = scopeStack.back();
        scopeStack.pop_back();
        return scope;
    }

};

class TypeResolver : Visitor{
private:
    void resolveFunctions(list<DefinedFunction*> funs);
    void resolve(Body& body);
    void resolveStatement(Statement* statement);
public:
    explicit TypeResolver(ErrorHandler& errorHandle, TypeTable* type_table);
    void resolve(AST& ast) override;
};

#endif //SPL_RESOLVER_H
