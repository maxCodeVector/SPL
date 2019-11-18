#ifndef __SPLAST__
#define __SPLAST__
#include "deliver.hpp"
#include "extnode.hpp"
#include "list"
using namespace std;


class AST: public BaseNode{
private:
    void convert2AST(AttrNode* root);
    void findEntity(BaseNode *extList);
    Scope* scope;


public:
    list<DefinedVariable*> vars;
    list<DefinedFunction*> functions;
    AST(AttrNode* root){
        convert2AST(root);
    }

    list<Entity*>& declaritions(list<Entity*>& decaries);
    void setConstant(ConstantTable &constantTable);
    list<DefinedFunction*>& defineFunctions();
    void setScope(Scope* scope_) override {
        this->scope = scope_;
    }

    ~AST();

};



#endif