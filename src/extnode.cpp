#include "extnode.hpp"

void BaseNode::setNext(AttrNode* extDef)
{
    BaseNode* base = extDef -> baseNode;
    if(base==NULL){
        return;
    }
    if(base->flag==VAR){
        auto var = (DefinedVariable*)base;
        next = var;
    } else if(base->flag==FUNC){
        auto* func = (DefinedFunction*)base;
        next  = base;
    }
}

DefinedVariable::DefinedVariable(AttrNode *spec, AttrNode *decList) {
    this->type = spec->firstChild->value;
    this->id = decList->firstChild->firstChild->value;
    this->flag = VAR;
}

AST& DefinedFunction::body(){


}

DefinedFunction::DefinedFunction(AttrNode *spec, AttrNode *fundec) {
    this->type = spec->firstChild->value;
    this->name = fundec->firstChild->value;
    this->flag = FUNC;
    getParameters(fundec);
}


