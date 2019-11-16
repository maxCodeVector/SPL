#ifndef __SPLSCOPE__
#define __SPLSCOPE__

#include <map>
#include <list>
#include <string>
#include "error.hpp"
using namespace std;

class Entity{

};

class DefinedVariable{

};

class ConstantTable{



};

class Scope{

protected:
    list<Scope> children; // the actual type is LocalScope, will be type convert

};

class ToplevelScope: public Scope{

protected:
    map<string, Entity> enties;
    list<DefinedVariable> staticLocalVariables; //cache
public:
    void declareEntity(Entity entity);
    void checkReferences(ErrorHandler &err);

};

class LocalScope: public Scope{

protected:
    Scope parent;
    map<string, DefinedVariable> variables;

};


#endif