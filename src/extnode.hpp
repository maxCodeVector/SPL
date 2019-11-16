#ifndef __EXTNODE__
#define __EXTNODE__
#include <string>
#include <list>
using namespace std;

class Entity{
public:
    string name;
    string type;
    string value;
    Entity(string &name){
        this->name = name;
    }

};

class Location{
public:
    string toString(){
        return "hh";
    }

};



class AST;
class Scope;

class BaseNode{
public:
    string name;
    virtual void setScope(Scope &scope){}
    virtual Location& location(){};


};

class ConstantEntry{
    string name;
    string value;

};

class ConstantTable{
    list<ConstantEntry> entries;


};

class DefinedVariable: public BaseNode{

};

class DefinedFunction: public BaseNode{
public:
    list<DefinedVariable> parameters();
    AST& body();

};



#endif