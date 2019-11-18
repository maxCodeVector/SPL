#include "semantic.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "ast.hpp"
using namespace std;


class LocalResolver{
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

    bool resolve(AST &ast){
        ToplevelScope* toplevelScope = new ToplevelScope();
        scopeStack.push_back(toplevelScope);

        list<Entity*> entities;
        for(Entity* e: ast.declaritions(entities)){
            Entity* hasE = toplevelScope->declareEntity(*e);
            if(hasE!= nullptr){
                string message = "duplicated define for:"+e->getName()+", last defined in:"+hasE->location()->toString();
                error(e->location(), message);
            }
        }

        resolveGloableVarIntializers();
        resolveConstantValues();
        resoveFunctions(ast.defineFunctions());
        toplevelScope->checkReferences(this->errorHandler);
        if(errorHandler.errorOccured()){
            return false;
        }
        ast.setScope(toplevelScope);
        ast.setConstant(this->constantTable);
        return true;
    }

    void resolve(Body& body){

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
                error(var->location(), message);
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

};

int semantic_analysis(AttrNode* root){
    AST* ast =  (AST*)root->baseNode;
    ErrorHandler h = ErrorHandler();
    LocalResolver local(h);
    local.resolve(*ast);
    h.showError();
    return 1;
}