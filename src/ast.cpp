#include "ast.hpp"


void AST::convert2AST(AttrNode* root){
    this->root = root;
    



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
