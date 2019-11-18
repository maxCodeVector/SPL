#ifndef __SPLSCOPE__
#define __SPLSCOPE__

#include <map>
#include <list>
#include <string>
#include "error.hpp"
#include "extnode.hpp"
using namespace std;

class LocalScope;
class Scope{

protected:
    list<LocalScope> children; // the actual type is LocalScope, will be type convert
public:
    virtual Entity* get(string& name){ return nullptr;};

};

class ToplevelScope: public Scope{

protected:
    map<string, Entity*> enties;
    list<DefinedVariable> staticLocalVariables; //cache
public:
    Entity* declareEntity(Entity &entity);
    void checkReferences(ErrorHandler &err);
    Entity* get(string& name);

};

class LocalScope: public Scope{

protected:
    Scope* parent;
    map<string, DefinedVariable> variables;

public:
    LocalScope(Scope *parent):Scope(){
        this->parent = parent;
    }
    bool isDefinedLocally(string& name);
    void defineVariable(DefinedVariable &var);
    Entity* get(string& name);


};


#endif