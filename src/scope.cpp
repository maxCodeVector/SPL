#include "scope.hpp"




void ToplevelScope::checkReferences(ErrorHandler &err){
    
}


bool ToplevelScope::declareEntity(Entity entity){
    int e = enties.count(entity.name);
    if(e==1){ // has existed
        return true;
    }
    enties.insert(pair<string, Entity>(entity.name, entity));
    return false;
}

bool LocalScope::isDefinedLocally(string& name){

}

void LocalScope::defineVariable(DefinedVariable &var){
    
}