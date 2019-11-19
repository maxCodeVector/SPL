#include "semantic.h"
#include "scope.hpp"
#include "error.h"
#include "ast.h"
#include <fstream>
using namespace std;


class LocalResolver:Visitor{
private:
    list<Scope*> scopeStack;
    ConstantTable constantTable;
    ErrorHandler& errorHandler;
    Scope* currentScope(){
        return scopeStack.back();
    }

public:
    LocalResolver(ErrorHandler &h): errorHandler(h), constantTable(), scopeStack(){

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
        resoveFunctions(ast.defineFunctions());
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

    void resoveFunctions(list<DefinedFunction*>& funcs){
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

    void error(Location* loc, string& message){
        this->errorHandler.recordError(loc, message);
    }

    void error(Error* err){
        if(!err)
            return;
        this->errorHandler.recordError(err);
    }

};

int semantic_analysis(AttrNode* root){
    AST* ast =  (AST*)root->baseNode;
    ErrorHandler h = ErrorHandler();
    LocalResolver local(h);
    local.resolve(*ast);
    h.showError(std::cerr);
    ofstream outfile("../src/res.o");
//    h.showError(outfile);
    delete(ast);
    return 1;
}