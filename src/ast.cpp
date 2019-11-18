#include "ast.hpp"

void AST::findEntity(BaseNode* extList){
    BaseNode* next = extList;
    while (next != nullptr){
        if(next->flag == FUNC){
            auto func = (DefinedFunction*)next;
            this->functions.push_back(func);
        } else if(next->flag == VAR){
            auto var = (DefinedVariable*)next;
            this->vars.push_back(var);
        }
        next = next->next;
    }
}

void AST::convert2AST(AttrNode* root){
    findEntity(root->baseNode);
}




list<Entity*>& AST::declaritions(list<Entity*>& decaries){
    for(DefinedVariable *var: this->vars){
        decaries.push_back(var);
    }
    for(DefinedFunction* function: this->functions){
        decaries.push_back(function);
    }
    return decaries;
}

list<DefinedFunction*>& AST::defineFunctions(){
    return this->functions;
}
void AST::setConstant(ConstantTable &constantTable){

}

AST::~AST() {
    free_all(vars);
    free_all(functions);
}


