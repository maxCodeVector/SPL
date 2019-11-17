#include "extnode.hpp"

void DefinedVariable::setNext(AttrNode* extDef)
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