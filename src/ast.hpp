#ifndef __SPLAST__
#define __SPLAST__
#include "deliver.hpp"
#include "extnode.hpp"
#include "list"
using namespace std;


class AST: public BaseNode{
    void convert2AST(AttrNode* root);
    list<Entity> declaritionList;
    struct AttrNode* root;


public:
    AST(AttrNode* root){
        convert2AST(root);
    }

    list<Entity> declaritions();
    void setConstant(ConstantTable &constantTable);
    list<DefinedFunction> defineFunctions();

};



#endif