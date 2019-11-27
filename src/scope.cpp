#include "scope.h"


Entity* ToplevelScope::declareEntity(Entity &entity){

    auto Itor = enties.find(entity.getName());

    if(Itor != enties.end()){ // has existed
        return Itor->second;
    }
    enties.insert(pair<string, Entity*>(entity.getName(), &entity));
    return nullptr;
}

Entity *ToplevelScope::get(string &name) {
    auto Itor = enties.find(name);

    if(Itor != enties.end()){ // has existed
        return Itor->second;
    } else
        return nullptr;
}

bool LocalScope::isDefinedLocally(string& name){
    auto Itor = variables.find(name);
    return Itor != variables.end();
}

void LocalScope::defineVariable(Variable &var){
    this->variables.insert(pair<string, Variable*>(var.getName(), &var));
}

Entity* LocalScope::get(string &name) {
    auto Itor = variables.find(name);

    if(Itor != variables.end()){ // has existed
        return Itor->second;
    }

    return parent->get(name);
}
