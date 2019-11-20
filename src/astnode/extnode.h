#ifndef __EXTNODE__
#define __EXTNODE__

#include "../includes.h"

class Scope;

class BaseNode {
private:
    Location *loc;
    bool loc_alloc;// mark is it allocate Location* loc

public:
    enum NodeType flag=OTHER;
    BaseNode *next;

    virtual void setScope(Scope *scope) {};

    Location *getLocation() {
        return loc;
    }

    void setLocation(Location* location) {
        this->loc = location;
        loc_alloc = false;
    }

    void setLocation(int lineNo, int colNo) {
        this->loc = new Location(lineNo, colNo);
        loc_alloc = true;
    }

    virtual void setNext(AttrNode *extDef);
    ~BaseNode(){
        if(loc_alloc)
            delete(loc);
    }

};

class Entity : public BaseNode{
public:
    bool isRefered = false;

    virtual string &getName()=0;

    void refere(){
        isRefered = true;
    }
};

class VariableType : public BaseNode {
    enum DataType type;
public:
    explicit VariableType(DataType type){
        this->type = type;
    }
    DataType getType(){
        return type;
    }
};
VariableType* getVariableType(string &name);


class DefinedVariable;
class Struct: public VariableType{
public:
    string typeName;
    list<DefinedVariable*> members;
    explicit Struct(AttrNode* name);
    Struct(AttrNode* name, AttrNode* defList);
    ~Struct();
    string& getTypeName(){
        return typeName;
    }
};

class DeclaredTypeVariable:public Entity{
    VariableType* type;
    string baseTypeName = "baseType";
public:
    explicit DeclaredTypeVariable(AttrNode* spec);
    string& getName() override {
        if(type->getType()==DataType::STRUCT_TYPE){
            return ((Struct*)type)->getTypeName();
        }
        return baseTypeName;
    }
};


class Exp;
class DefinedVariable : public Entity {
private:
    VariableType* type;
    string id;
    Exp* value;
    list<int > array;
public:

    explicit DefinedVariable(AttrNode *varDec);

    void setType(AttrNode *spec);
    void addDimension(AttrNode *dim);
    void setExp(AttrNode* exp){
        this->value = (Exp*)exp->baseNode;
    }
    Exp* getValue(){
        return value;
    };

    string & getName() override {
        return this->id;
    }

    ~DefinedVariable();

};

class Body;
class DefinedFunction : public Entity {
private:
    VariableType* returnType;
    string id;
    list<DefinedVariable*> parameters;
    Body* functionBody;
    void parseParameters(AttrNode* paraList);

public:
    DefinedFunction(AttrNode *functionID, AttrNode *paraList);
    explicit DefinedFunction(AttrNode *functionID);
    ~DefinedFunction();


    list<DefinedVariable*>& getParameters();
    void setReturnType(AttrNode* type);

    Body *getBody();
    void setBody(AttrNode* body){
        this->functionBody = (Body*)body->baseNode;
    }

    string &getName() override {
        return this->id;
    }

};


class Args: public BaseNode{
public:
    list<Exp*> args;
    ~Args();
};


#endif