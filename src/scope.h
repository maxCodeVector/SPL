#ifndef __SPLSCOPE__
#define __SPLSCOPE__

#include <map>
#include <list>
#include <string>
#include "error.h"
#include "astnode/extnode.h"
#include "type.h"
using namespace std;

class LocalScope;
class Scope{
public:
    list<LocalScope*> children; // the actual type is LocalScope, will be type convert
    virtual Entity* get(string& name)=0;
    virtual ~Scope(){
        free_all(children);
    }


};

class ToplevelScope: public Scope{

protected:
    map<string, Entity*> enties;
    TypeTable typeTable;
    list<DefinedVariable*> staticLocalVariables; //cache
public:
    Entity* declareEntity(Entity &entity);
    void  declareVariableType(VariableType* variableType, ErrorHandler &err){
        typeTable.declareVariableType(variableType, err);
    }
    void checkReferences(ErrorHandler &err);
    Entity* get(string& name) override ;
    VariableType* queryType(string& name){
        typeTable.queryType(name);
    }

};

class LocalScope: public Scope{

protected:
    Scope* parent;
    map<string, DefinedVariable*> variables;

public:
    LocalScope(Scope *parent):Scope(){
        this->parent = parent;
    }
    bool isDefinedLocally(string& name);
    void defineVariable(DefinedVariable &var);
    Entity* get(string& name);


};


#endif