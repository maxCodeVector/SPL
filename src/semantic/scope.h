#ifndef __SPLSCOPE__
#define __SPLSCOPE__

#include <map>
#include <list>
#include <string>
#include "../error.h"
#include "../astnode/extnode.h"
#include "type.h"
using namespace std;

class LocalScope;
class Scope{
public:
    list<LocalScope*> children; // the actual elementType is LocalScope, will be elementType convert
    virtual Entity* get(string& name)=0;
    virtual ~Scope(){
        free_all(children);
    }


};

class ToplevelScope: public Scope{

protected:
    map<string, Entity*> enties;
    list<Variable*> staticLocalVariables; //cache
public:
    Entity* declareEntity(Entity &entity);
    void checkReferences(ErrorHandler &err);
    Entity* get(string& name) override ;

};

class LocalScope: public Scope{

protected:
    Scope* parent;
    map<string, Variable*> variables;

public:
    LocalScope(Scope *parent):Scope(){
        this->parent = parent;
    }
    bool isDefinedLocally(string& name);
    void defineVariable(Variable &var);
    map<string, Variable*>& getVariables(){
        return variables;
    }
    Entity* get(string& name);


};


#endif