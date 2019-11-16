#include "sematic.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "ast.hpp"
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

    bool resove(AST &ast){
        ToplevelScope toplevelScope;
        scopeStack.push_back(toplevelScope);

        for(Entity e: ast.declaritions()){
            toplevelScope.declareEntity(e);
        }

        resoveFunctions(ast.defineFunctions());
        toplevelScope.checkReferences(this->errorHandler);
        if(errorHandler.errorOccured()){
            return false;
        }
        ast.setScope(toplevelScope);
        ast.setConstant(this->constantTable);
        return true;
    }

    void resoveGloableVarIntializers();
    void resoveConstantValues();
    void resoveFunctions(list<DefinedFunction> funcs){
        for(DefinedFunction function: funcs){
            pushScope(function.parameters());
            resove(function.body());
            function.setScope(popScope());
        }
    }

    void pushScope(list<DefinedVariable> vars){
        LocalScope scope = LocalScope(currentScope());
        for(DefinedVariable var: vars){
            if(scope.isDefinedLocally(var.name)){
                error(var.location(), "duplicated variable in scope:"+var.name);
            }else{
                scope.defineVariable(var);
            }
        }
        scopeStack.push_back(scope);
    }

    Scope& popScope(){
        Scope& scope = scopeStack.back();
        scopeStack.pop_back();
        return scope;
    }

    void error(Location& loc, string message){
        fprintf(stderr, "error in:%s, %s", loc.toString().c_str(), message.c_str());
    }



};





int sematic_analysis(AttrNode* root){
    AST ast =  AST(root);
    ErrorHandler h = ErrorHandler();
    LocalResolver local(h);
    local.resove(ast);
    return 1;
}