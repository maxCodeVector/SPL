#include "sematic.h"
#include "scope.hpp"
#include "error.hpp"
#include "ast.hpp"
using namespace std;


class LocalResolver{
private:
    list<Scope> scopeStack;
    ConstantTable constantTable;
    ErrorHandler& errorHandler;

public:
    LocalResolver(ErrorHandler &h): errorHandler(h), constantTable(), scopeStack(){

    }

    bool resove(AST &ast){
        ToplevelScope toplevelScope;
        scopeStack.push_back((Scope)toplevelScope);

        for(Entity e: ast.declaritions()){
            toplevelScope.declareEntity(e);
        }

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
    void resoveFunctions();



};





int sematic_analysis(AttrNode* root){
    AST ast =  AST(root);
    ErrorHandler h = ErrorHandler();
    LocalResolver local(h);
    local.resove(ast);
    return 1;
}