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




list<Entity>& AST::declaritions(list<Entity>& decaries){
    for(DefinedVariable var: this->vars){
        Entity* entity = new Entity(var.name());
        entity->location = var.location();
        decaries.push_back(*entity);
    }
    for(DefinedFunction function: this->functions){
        Entity* entity = new Entity(function.name());
        entity->location = function.location();
        decaries.push_back(*entity);
    }
    return decaries;
}

list<DefinedFunction>* AST::defineFunctions(){
    return &this->functions;
}
void AST::setConstant(ConstantTable &constantTable){


    
}


