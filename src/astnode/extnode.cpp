#include "extnode.h"
#include "../scope.h"
#include "statement.h"
#include "expression.h"

VariableType* getVariableType(string &name) {
    DataType type;
    if (name == "int") {
        return new VariableType(INT_TYPE);
    } else if (name == "float") {
        return new VariableType(FLOAT_TYPE);
    } else if (name == "char") {
        return new VariableType(CHAR_TYPE);
    } else
        return new VariableType(STRUCT_TYPE);
}

void setNextNode(BaseNode* pre, BaseNode* next){
    if(pre->next!= nullptr){
        setNextNode(pre->next, next);
        return;
    }
    if (next->flag == VAR) {
        auto var = (DefinedVariable *) next;
        pre->next = var;
    } else if (next->flag == FUNC) {
        auto *func = (DefinedFunction *) next;
        pre->next = func;
    } else
        pre->next = next;
}


void BaseNode::setNext(AttrNode *extDef) {
    BaseNode *base = extDef->baseNode;
    if (base == nullptr) {
        return;
    }
    setNextNode(this, base);
}


DefinedVariable::DefinedVariable(AttrNode *varDec) {
    this->id = varDec->value;
    setLocation(new Location(varDec->lineNo, 0));
    this->flag = VAR;
}

void DefinedVariable::setType(AttrNode *spec) {
    VariableType* type = (VariableType*)spec->baseNode;
    DefinedVariable* next = this;
    this->is_type_allocated = true;
    while (next!=nullptr){
        next->type = type;
        next = (DefinedVariable*)next->next;
    }
//    delete(type);
}

void DefinedVariable::addDimension(AttrNode* dim) {
    int newDim = stoi(dim->value);
    this->array.push_back(newDim);
}

DefinedVariable::~DefinedVariable() {
    if(is_type_allocated)
        delete(type);
    delete(value);
}

DefinedFunction::DefinedFunction(AttrNode *functionID, AttrNode *paraList) {
    this->id = functionID->value;
    setLocation(new Location(functionID->lineNo, 0));
    this->flag = FUNC;
    parseParameters(paraList);
}

DefinedFunction::DefinedFunction(AttrNode *functionID) {
    this->id = functionID->value;
    setLocation(new Location(functionID->lineNo, 0));
    this->flag = FUNC;
}

void DefinedFunction::parseParameters(AttrNode* paraList) {
    DefinedVariable* var = (DefinedVariable*)paraList->baseNode;
    while (var!= nullptr){
        this->parameters.push_back(var);
        var = (DefinedVariable*)var->next;
    }
}


list<DefinedVariable*>& DefinedFunction::getParameters() {
    return this->parameters;
}

Body* DefinedFunction::getBody() {
    return this->functionBody;
}

void DefinedFunction::setReturnType(AttrNode *type) {
    this->returnType = (VariableType*)type->baseNode;
}

DefinedFunction::~DefinedFunction() {
    delete(returnType);
    delete(functionBody);
    free_all(parameters);
}


Struct::~Struct() {
    free_all(members);
}

Struct::Struct(AttrNode *name):VariableType(STRUCT_TYPE){
    this->typeName = name->value;
    this->setLocation(name->lineNo, 0);
    is_complete = false;
}

Struct::Struct(AttrNode *name, AttrNode *defList):VariableType(STRUCT_TYPE){
    this->typeName = name->value;
    DefinedVariable* nextVar = (DefinedVariable*)defList->baseNode;
    while (nextVar!= nullptr){
        this->members.push_back(nextVar);
        nextVar = (DefinedVariable*)nextVar->next;
    }
    this->setLocation(name->lineNo, 0);
    is_complete = true;
}


DeclaredTypeVariable::DeclaredTypeVariable(AttrNode *spec){
    this->flag = DECLARATION;
    VariableType* variableType = (VariableType*)spec->baseNode;
    this->type = variableType;
}

Args::~Args() {
    free_all(args);
}
