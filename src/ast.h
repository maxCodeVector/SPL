#ifndef __SPLAST__
#define __SPLAST__
#include "deliver.h"
#include "scope.h"

class AST{
    void convert2AST(AttrNode* root);

public:
    AST(AttrNode* root){
        convert2AST(root);
    }

    list<Entity> declaritions();
    void setScope(Scope &scope);
    void setConstant(ConstantTable &constantTable);

 

};




#endif