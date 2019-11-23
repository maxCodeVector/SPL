#ifndef __SPLAST__
#define __SPLAST__
#include <list>
#include "astnode/statement.h"
#include "astnode/expression.h"
using namespace std;

class ToplevelScope;
class AST: public BaseNode{
private:
    void convert2AST(AttrNode* root);
    void findEntity(BaseNode *extList);
    ToplevelScope* toplevelScope;
    list<DefinedFunction*> functions;
    list<DefinedVariable*> vars;
    list<VariableType*> declaredTypes;

public:
    AST(AttrNode* root){
        convert2AST(root);
    }
    int showSize();
    list<Entity*>& declaritions(list<Entity*>& decaries);
    void setConstant(ConstantTable &constantTable);
    list<DefinedFunction*>& defineFunctions();
    list<DefinedVariable*>& getDefinedVars(){
        return vars;
    }
    list<VariableType*>& getDeclaredTypes(){
        return declaredTypes;
    }
    ToplevelScope* getScope(){
        return toplevelScope;
    }

    void setScope(Scope* scope_) override;

    ~AST();

};

class TypeTable;
class Visitor{
    virtual void resolve(AST& ast)=0;

protected:
    ErrorHandler& errorHandler;
    TypeTable* typeTable;

public:
    explicit Visitor(ErrorHandler& h, TypeTable* type_table):errorHandler(h){
        this->typeTable = type_table;
    };
    void error(Location* loc, ErrorType errorType, string& message){
        this->errorHandler.recordError(loc, errorType, message);
    }

    void error(Error* err){
        if(!err)
            return;
        this->errorHandler.recordError(err);
    }
};


#endif