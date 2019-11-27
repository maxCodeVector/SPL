#ifndef __EXTNODE__
#define __EXTNODE__

#include <map>
#include "../includes.h"

class Scope;

class BaseNode {
private:
    Location *loc;
    bool loc_alloc;// mark is it allocate Location* loc

public:
    enum NodeType flag = OTHER;
    BaseNode *next;

    virtual void setScope(Scope *scope) {};

    Location *getLocation() {
        if (loc == nullptr) {
            exit(ErrorCode::NULL_LOCATION);
        }
        return loc;
    }

    void setLocation(Location *location) {
        this->loc = location;
        loc_alloc = false;
    }

    void setLocation(int lineNo, int colNo) {
        this->loc = new Location(lineNo, colNo);
        loc_alloc = true;
    }

    virtual void setNext(AttrNode *extDef);

    ~BaseNode() {
        if (loc_alloc)
            delete (loc);
    }

};

class Entity : public BaseNode {
public:
    bool isRefered = false;

    virtual string &getName() = 0;

    void refere() {
        isRefered = true;
    }
};

class VariableType : public BaseNode {
    string baseTypeName = "baseType";
    VariableType* element;
    enum DataType elementType;// primitive or temp_array_size, struct
    // if elementType is primitive elementType, then element is NULL
    int elementNum = 0;
public:
    int getArraySize() const {
        return elementNum;
    }

public:
    explicit VariableType(DataType type) {
        this->elementType = type;
    }

    explicit VariableType(VariableType* element, int size) {
        this->element = element;
        this->elementNum = size;
        this->elementType = ARRAY_TYPE;
    }

    VariableType *getElement() const {
        return element;
    }

    DataType getElementType() {
        return elementType;
    }

    virtual string &getTypeName() {
        return baseTypeName;
    }

    virtual bool isComplete() {
        return true;
    }

    virtual void setActualType(VariableType *type) {}

    virtual VariableType *getActualType() {
        return this;
    }

    bool isArray(){
        return elementNum > 0;
    }

};

VariableType *getVariableType(string &name);


class Variable;

class Struct : public VariableType {
    string typeName;
    bool is_complete;
    VariableType *actualType; // used for if elementType is not complete
    list<Variable *> members;
    map<string, Variable *> memberMap;

public:
    explicit Struct(AttrNode *name);

    Struct(AttrNode *name, AttrNode *defList);
    Struct(const Struct& aStruct):VariableType(STRUCT_TYPE){
        this->typeName = aStruct.typeName;
        this->is_complete = aStruct.is_complete;

    }

    ~Struct();

    string &getTypeName() override {
        return typeName;
    }

    bool isComplete() override {
        return is_complete;
    }

    void setActualType(VariableType *type) override {
        actualType = type;
    }

    VariableType *getActualType() override {
        if (is_complete || actualType == nullptr)
            return this;
        return actualType;
    }

    list<Variable *> &getMemberList() {
        return members;
    };

    Variable *getMember(string &name);

    Error *checkDuplicatedNameMember();


};

class DeclaredVariableType : public Entity {
    VariableType *type;
public:
    explicit DeclaredVariableType(AttrNode *spec);

    string &getName() override {
        return type->getTypeName();
    }

    VariableType *getType() {
        return type;
    }
};


class Exp;

class Variable : public Entity {
private:
    VariableType* type;
    list<int > temp_array_size;
    string id;
    Exp *value;
public:

    explicit Variable(AttrNode *varDec);

    void setType(AttrNode *spec);

    VariableType *getType() {
        return type->getActualType();
    }

    void addDimension(AttrNode *dim);

    void setExp(AttrNode *exp) {
        this->value = (Exp *) exp->baseNode;
    }

    Exp *getValue() {
        return value;
    };

    void setArray(VariableType* elementType);

//    list<int> &getArrayDimension() {
//        return temp_array_size;
//    };


    /**
     * indicate if it is array when I am in this dimension
     * @param dimension, normal input dimension is 0
     * @return
     */
    bool isArray(int dimension) {
        VariableType* inner_element = this->type;
        while (dimension > 0){
            if(inner_element== nullptr){
                return false;
            }
            inner_element = inner_element->getElement();
            dimension--;
        }
        return inner_element->isArray();
    }

    string &getName() override {
        return this->id;
    }

    ~Variable();

};

class Body;

class Function : public Entity {
private:
    VariableType *returnType;
    string id;
    list<Variable *> parameters;
    Body *functionBody;

    void parseParameters(AttrNode *paraList);

public:
    Function(AttrNode *functionID, AttrNode *paraList);

    explicit Function(AttrNode *functionID);

    ~Function();


    list<Variable *> &getParameters();

    void setReturnType(AttrNode *type);

    VariableType *getReturnType() {
        return returnType;
    }

    Body *getBody();

    void setBody(AttrNode *body) {
        this->functionBody = (Body *) body->baseNode;
    }

    string &getName() override {
        return this->id;
    }

};


class Args : public BaseNode {
public:
    list<Exp *> args;

    Args(Location *loc) {
        setLocation(loc);
    }

    ~Args();

    list<Exp *> &getArguments() {
        return args;
    }
};


#endif