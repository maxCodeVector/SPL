#include "extnode.h"
#include "../semantic/scope.h"
#include "statement.h"
#include "expression.h"

VariableType *getVariableType(string &name) {
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

void setNextNode(BaseNode *pre, BaseNode *next) {
    if (pre->next != nullptr) {
        setNextNode(pre->next, next);
        return;
    }
    if (next->flag == VAR) {
        auto var = (Variable *) next;
        pre->next = var;
    } else if (next->flag == FUNC) {
        auto *func = (Function *) next;
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


Variable::Variable(AttrNode *varDec) {
    this->id = varDec->value;
    setLocation(varDec->lineNo, 0);
    this->flag = VAR;
}

void Variable::setType(AttrNode *spec) {
    VariableType *type = (VariableType *) spec->baseNode;
    Variable *next = this;
    while (next != nullptr) {
        next->setArray(type);
        next = (Variable *) next->next;
    }
    delete (type);
}

void Variable::addDimension(AttrNode *dim) {
    int newDim = stoi(dim->value);
    this->temp_array_size.push_back(newDim);
}

Variable::~Variable() {
    delete (value);
    delete (type);
}

void Variable::setArray(VariableType *element) {
    if (element->getElementType() == ARRAY_TYPE) {
        printf("a wsl\n");
        exit(ErrorCode::ARRAY_TYPE_ARRAY);
    }
    VariableType *inner_element;
    if (element->getElementType() == STRUCT_TYPE) {
        inner_element = new Struct(*(Struct *) element);
    } else
        inner_element = new VariableType(element->getElementType());
    auto itor = temp_array_size.end();
    while (itor != temp_array_size.begin()) {
        itor--;
        inner_element = new VariableType(inner_element, *itor);
    }
    this->type = inner_element;
}

Variable::Variable(const char *id, DataType type) {
    this->id = id;
    this->type = new VariableType(type);
}

void Variable::setIR(IRInst *irInst) {

}

Function::Function(AttrNode *functionID, AttrNode *paraList) {
    this->id = functionID->value;
    setLocation(functionID->lineNo, 0);
    this->flag = FUNC;
    parseParameters(paraList);
}

Function::Function(AttrNode *functionID) {
    this->id = functionID->value;
    setLocation(functionID->lineNo, 0);
    this->flag = FUNC;
}

void Function::parseParameters(AttrNode *paraList) {
    Variable *var = (Variable *) paraList->baseNode;
    while (var != nullptr) {
        this->parameters.push_back(var);
        var = (Variable *) var->next;
    }
}


list<Variable *> &Function::getParameters() {
    return this->parameters;
}

Body *Function::getBody() {
    return this->functionBody;
}

void Function::setReturnType(AttrNode *type) {
    this->returnType = (VariableType *) type->baseNode;
}

Function::~Function() {
    delete (returnType);
    delete (functionBody);
    free_all(parameters);
}

Function* getBuildFunction(const char* id, VariableType* returnType){
    Function* function = new Function;
    function->id = id;
    function->returnType = returnType;
    function->flag = BUILD_NODE;
    function->setLocation(-1, 0);
    return function;
}

void Function::setIR(list<IRStatement *> *) {

}


Struct::~Struct() {
    free_all(members);
}

Struct::Struct(AttrNode *name) : VariableType(STRUCT_TYPE) {
    this->typeName = name->value;
    this->setLocation(name->lineNo, 0);
    is_complete = false;
}

Struct::Struct(AttrNode *name, AttrNode *defList) : VariableType(STRUCT_TYPE) {
    this->typeName = name->value;
    Variable *nextVar = (Variable *) defList->baseNode;
    while (nextVar != nullptr) {
        this->members.push_back(nextVar);
        nextVar = (Variable *) nextVar->next;
    }
    this->setLocation(name->lineNo, 0);
    is_complete = true;
}

Error *Struct::checkDuplicatedNameMember() {
    for (Variable *var: members) {
        string &name = var->getName();
        auto itor = memberMap.find(name);
        if (itor != memberMap.end()) {
            return new Error{var->getLocation(), OTHER_ERROR, "struct elementType members should have different name"};
        }
        memberMap.insert(pair<string, Variable *>(name, var));
    }
    return nullptr;
}

Variable *Struct::getMember(string &name) {
    auto itor = memberMap.find(name);
    if (itor == memberMap.end())
        return nullptr;
    return itor->second;
}


DeclaredVariableType::DeclaredVariableType(AttrNode *spec) {
    this->flag = DECLARATION;
    VariableType *variableType = (VariableType *) spec->baseNode;
    this->type = variableType;
}

Args::~Args() {
    free_all(args);
}
