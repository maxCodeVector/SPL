#include "scope.hpp"




void ToplevelScope::checkReferences(ErrorHandler &err){
    
}


Entity* ToplevelScope::declareEntity(Entity entity){

    auto Itor = enties.find(entity.name);

    if(Itor != enties.end()){ // has existed
        return &Itor->second;
    }
    enties.insert(pair<string, Entity>(entity.name, entity));
    return nullptr;
}

bool LocalScope::isDefinedLocally(string& name){

}

void LocalScope::defineVariable(DefinedVariable &var){
    
}