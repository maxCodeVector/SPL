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

};

class ToplevelScope: public Scope{

protected:
    map<string, Entity> enties;
    list<DefinedVariable> staticLocalVariables; //cache
public:
    bool declareEntity(Entity entity);
    void checkReferences(ErrorHandler &err);

};

class LocalScope: public Scope{

protected:
    Scope parent;
    map<string, DefinedVariable> variables;

public:
    LocalScope(Scope &scope):Scope(){

    }
    bool isDefinedLocally(string& name);
    void defineVariable(DefinedVariable &var);


};


#endif