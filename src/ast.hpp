#ifndef __SPLAST__
#define __SPLAST__
#include "deliver.hpp"
#include "extnode.hpp"
#include "list"
using namespace std;


class AST: public BaseNode{
private:
    void convert2AST(AttrNode* root);
    list<Entity> declaritionList;
    struct AttrNode* root;


public:
    list<DefinedVariable> vars;
    list<DefinedFunction> functions;
    AST(AttrNode* root){
        convert2AST(root);
    }

    list<Entity> declaritions();
    void setConstant(ConstantTable &constantTable);
    list<DefinedFunction> defineFunctions();

};



#endif