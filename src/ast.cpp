#include "error.h"
#include "scope.h"
#include "ast.h"

void AST::findEntity(BaseNode* extList){
    BaseNode* next = extList;
    while (next != nullptr){
        if(next->flag == FUNC){
            auto func = (DefinedFunction*)next;
            this->functions.push_back(func);
        } else if(next->flag == VAR){
            auto var = (DefinedVariable*)next;
            this->vars.push_back(var);
        } else if(next->flag==DECLARATION){
            auto  dec = (DeclaredTypeVariable*)next;
            string& decName = dec->getName();
            if("baseType"!=decName) {
                this->declaredTypes.push_back(dec->getType());
            }
        }
        next = next->next;
    }
}

void AST::convert2AST(AttrNode* root){
    findEntity(root->baseNode);
}

int AST::showSize() {
    return declaredTypes.size();
}

void AST::setScope(Scope *scope_) {
    this->toplevelScope = (ToplevelScope*)scope_;
}

void AST::setTypeTable(TypeTable *table) {
    typeTable = table;
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
    free_all(declaredTypes);
    delete(toplevelScope);
    delete (typeTable);
}



