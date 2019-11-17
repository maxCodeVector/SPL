#include "ast.hpp"

void findEntity(AttrNode* node){
    AttrNode* firstchild = node->firstChild;
    AttrNode* child = firstchild;
    while (child!=NULL)
    {
        findEntity(child);
        child = child->nextSibling;
    }
    
}

void AST::convert2AST(AttrNode* root){
    this->root = root;

    findEntity(root);
    



}




list<Entity> AST::declaritions(){
    list<Entity> decaries;
    return this->declaritionList;
}

list<DefinedFunction> AST::defineFunctions(){
    list<DefinedFunction> defineFunctionList;
    return defineFunctionList;
}
void AST::setConstant(ConstantTable &constantTable){


    
}

list<DefinedVariable> DefinedFunction::parameters(){

}

AST& DefinedFunction::body(){
    

}
