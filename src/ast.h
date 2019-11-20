#ifndef __SPLAST__
#define __SPLAST__
#include "list"
#include "astnode/statement.h"
#include "astnode/expression.h"
using namespace std;


class AST: public BaseNode{
private:
    void convert2AST(AttrNode* root);
    void findEntity(BaseNode *extList);
    Scope* scope;
    list<DefinedFunction*> functions;


public:
    list<DefinedVariable*> vars;
    AST(AttrNode* root){
        convert2AST(root);
    }

    list<Entity*>& declaritions(list<Entity*>& decaries);
    void setConstant(ConstantTable &constantTable);
    list<DefinedFunction*>& defineFunctions();
    list<DefinedVariable*>& getDefinedVars(){
        return vars;
    }

    void setScope(Scope* scope_) override {
        this->scope = scope_;
    }

    ~AST();

};

class Visitor{
    virtual void resolve(AST& ast)=0;
    virtual void resolve(Body& body)=0;

protected:
    ErrorHandler& errorHandler;
public:
    explicit Visitor(ErrorHandler& h):errorHandler(h){};
    void error(Location* loc, string& message){
        this->errorHandler.recordError(loc, message);
    }

    void error(Error* err){
        if(!err)
            return;
        this->errorHandler.recordError(err);
    }
};


#endif