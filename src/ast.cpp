#include "ast.hpp"

void AST::findEntity(BaseNode* extList){
    BaseNode* next = extList;
    while (next != nullptr){
        if(next->flag == FUNC){
            auto func = (DefinedFunction*)next;
            this->functions.push_back(*func);
        } else if(next->flag == VAR){
            auto var = (DefinedVariable*)next;
            this->vars.push_back(*var);
        }
        next = next->next;
    }
}

void AST::convert2AST(AttrNode* root){
    this->root = root;
    findEntity(root->baseNode);
    



}




list<Entity> AST::declaritions(){
    list<Entity> decaries;
    return decaries;
}

list<DefinedFunction> AST::defineFunctions(){
    list<DefinedFunction> defineFunctionList;
    return defineFunctionList;
}
void AST::setConstant(ConstantTable &constantTable){


    
}

list<DefinedVariable> DefinedFunction::parameters(){

}


