#include "sematic.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "ast.hpp"
#include <iostream>
using namespace std;


class LocalResolver{
private:
    list<Scope> scopeStack;
    ConstantTable constantTable;
    ErrorHandler& errorHandler;
    Scope& currentScope(){
        return scopeStack.back();
    }

public:
    LocalResolver(ErrorHandler &h): errorHandler(h), constantTable(), scopeStack(){

    }

    bool resolve(AST &ast){
        ToplevelScope toplevelScope;
        scopeStack.push_back(toplevelScope);

        list<Entity> entities;
        for(Entity &e: ast.declaritions(entities)){
            Entity* hasE = toplevelScope.declareEntity(e);
            if(hasE!= nullptr){
                string message = "duplicated define for:"+e.name+", last defined in:"+hasE->location->toString();
                error(e.location, message);
            }
        }

        resoveFunctions(*ast.defineFunctions());
        toplevelScope.checkReferences(this->errorHandler);
        if(errorHandler.errorOccured()){
            return false;
        }
        ast.setScope(toplevelScope);
        ast.setConstant(this->constantTable);
        return true;
    }

    void resolve(Body& body){

    }


    void resoveGloableVarIntializers();
    void resoveConstantValues();
    void resoveFunctions(list<DefinedFunction>& funcs){
        for(DefinedFunction& function: funcs){
            pushScope(*function.getParameters());
            resolve(*function.getBody());
            function.setScope(popScope());
        }
    }


    void pushScope(list<DefinedVariable>& vars){
        LocalScope* scope = new LocalScope(currentScope());
        for(DefinedVariable& var: vars){
            if(scope->isDefinedLocally(var.name())){
                string message = "fuck：" + var.name();
                error(var.location(), message);
            }else{
                scope->defineVariable(var);
            }
        }
        scopeStack.push_back(*scope);
    }

    Scope& popScope(){
        Scope& scope = scopeStack.back();
        scopeStack.pop_back();
        return scope;
    }

    void error(Location* loc, string& message){
        std::cerr << "error in:" << loc->toString() << ", " << message << endl;
    }



};





int sematic_analysis(AttrNode* root){
    AST* ast =  (AST*)root->baseNode;
    ErrorHandler h = ErrorHandler();
    LocalResolver local(h);
    local.resolve(*ast);
    return 1;
}