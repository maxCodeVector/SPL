#ifndef __EXTNODE__
#define __EXTNODE__
#include <string>
#include <list>
#include "deliver.hpp"
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
enum NodeType{
    VAR,
    FUNC
};

class BaseNode{
public:
    enum NodeType flag;
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
private:
    string type;
    string id;
    string value;
    BaseNode* next;
public:
    DefinedVariable(AttrNode* spec, AttrNode* decList){
        this->type = spec->firstChild->value;
        this->id = decList->firstChild->firstChild->value;
        this->flag = VAR;
    }
    void setNext(AttrNode* extDef);
    string& name(){
        return this->id;
    }


};

class DefinedFunction: public BaseNode{
private:
    string type;
    string name;
    list<Entity> paras;
    void getParameters(AttrNode* fundec){

    }

public:
    DefinedFunction(AttrNode* spec, AttrNode* fundec){
        this->type = spec->firstChild->value;
        this->name = fundec->firstChild->value;
        this->flag = FUNC;
        getParameters(fundec);
    }
    list<DefinedVariable> parameters();
    AST& body();

};



#endif