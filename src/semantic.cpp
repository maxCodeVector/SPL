#include "semantic.h"
#include "scope.h"
#include "error.h"
#include <fstream>
#include "type.h"
#include "ast.h"

using namespace std;


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
        ToplevelScope* toplevelScope = (ToplevelScope*)scopeStack.front();
        while (!scopeStack.empty()) {
            scopeStack.pop_front();
        }
        delete(toplevelScope);
    }



    void resolve(AST& ast){
        ToplevelScope* toplevelScope = new ToplevelScope();
        scopeStack.push_back(toplevelScope);

        list<Entity*> entities;
        for(Entity* e: ast.declaritions(entities)){
            Entity* hasE = toplevelScope->declareEntity(*e);
            if(hasE!= nullptr){
                string message = "duplicated define for:"+e->getName()+", last defined in:"+ hasE->getLocation()->toString();
                error(e->getLocation(), message);
            }
        }

        resolveGloableVarIntializers();
        resolveConstantValues();
        resolveFunctions(ast.defineFunctions());
        toplevelScope->checkReferences(this->errorHandler);
        if(errorHandler.errorOccured()){
            return;
        }
        ast.setScope(toplevelScope);
        ast.setConstant(this->constantTable);
    }

    void resolve(Body& body){
        LocalScope* curr = (LocalScope*)currentScope();
        for(DefinedVariable* var: body.vars){
            if(curr->isDefinedLocally(var->getName())){
                string message = "duplicated variable in scope：" + var->getName();
                error(var->getLocation(), message);
            }else{
                curr->defineVariable(*var);
            }
        }
        for(DefinedVariable* var: body.vars){
            if(var->getValue()){
                error(var->getValue()->checkReference(currentScope()));
            }
        }
        for (Statement* statement:body.statements){
            resolve(*statement);
        }
    }

    void resolve(Statement& statement){
        Exp* exp = statement.getExpression();
        Error* _error = exp->checkReference(currentScope());
        if(_error){
            error(_error);
        }
    }

    void resolveGloableVarIntializers(){

    }
    void resolveConstantValues(){

    }

    void resolveFunctions(list<DefinedFunction*>& funcs){
        for(DefinedFunction* function: funcs){
            pushScope(function->getParameters());
            resolve(*function->getBody());
            function->setScope(popScope());
        }
    }


    void pushScope(list<DefinedVariable*>& vars){
        Scope* parent = currentScope();
        LocalScope* scope = new LocalScope(parent);
        parent->children.emplace_back(scope);

        for(DefinedVariable* var: vars){
            if(scope->isDefinedLocally(var->getName())){
                string message = "duplicated variable in scope：" + var->getName();
                error(var->getLocation(), message);
            }else{
                scope->defineVariable(*var);
            }
        }
        scopeStack.push_back(scope);
    }

    Scope* popScope(){
        Scope* scope = scopeStack.back();
        scopeStack.pop_back();
        return scope;
    }

};


int semantic_analysis(AST &ast) {
    ErrorHandler h = ErrorHandler();
    LocalResolver local(h);
    local.resolve(ast);
    DereferenceChecker typeResolver(h);
    typeResolver.resolve(ast);

    h.showError(std::cerr);
    ofstream outfile("../src/res.o");
//    h.showError(outfile);
    return 1;
}