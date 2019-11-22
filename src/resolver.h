//
// Created by hya on 11/22/19.
//

#ifndef SPL_RESOLVER_H
#define SPL_RESOLVER_H

#include "scope.h"
#include "ast.h"

class LocalResolver:Visitor{
private:
    list<Scope*> scopeStack;
    ConstantTable constantTable;

    Scope* currentScope(){
        return scopeStack.back();
    }

public:
    explicit LocalResolver(ErrorHandler &h): Visitor(h), constantTable(), scopeStack(){
    }
    ~LocalResolver(){
        while (!scopeStack.empty()) {
            scopeStack.pop_front();
        }
    }


    void resolveDeclaredType(ToplevelScope* toplevel, list<VariableType*> & declared);

    void resolve(AST& ast) override;

    void resolve(Body& body) override;

    void resolve(Statement& statement);

    void resolveGloableVarIntializers(){

    }
    void resolveConstantValues(){

    }

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
    ToplevelScope* toplevelScope;
    void resolveFunctions(list<DefinedFunction*> funs);
    void resolve(Body& body) override;
    void resolveStatement(Statement* statement);
public:
    void resolve(AST& ast) override;
    explicit TypeResolver(ErrorHandler& errorHandle);
};

#endif //SPL_RESOLVER_H
