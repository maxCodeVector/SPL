#include "extnode.hpp"


static VariableType intType(DataType::INT_TYPE);
static VariableType floatType(DataType::FLOAT_TYPE);
static VariableType charType(DataType::CHAR_TYPE);
VariableType* getVariableType(string &name) {
    DataType type;
    if (name == "int") {
        return &intType;
    } else if (name == "float") {
        return &floatType;
    } else if (name == "char") {
        return &charType;
    } else
        return new VariableType(STRUCT_TYPE);
}

void BaseNode::setNext(AttrNode *extDef) {
    BaseNode *base = extDef->baseNode;
    if (base == NULL) {
        return;
    }
    if (base->flag == VAR) {
        auto var = (DefinedVariable *) base;
        next = var;
    } else if (base->flag == FUNC) {
        auto *func = (DefinedFunction *) base;
        next = func;
    } else
        next = base;
}

DefinedVariable::DefinedVariable(AttrNode *spec, AttrNode *decList) {
    this->type = getVariableType(spec->firstChild->value);
    this->id = decList->firstChild->firstChild->value;
    this->loc = new Location(spec->lineNo, 0);
    this->flag = VAR;
}


DefinedVariable::DefinedVariable(AttrNode *varDec) {
    this->id = varDec->value;
    this->loc = new Location(varDec->lineNo, 0);
    this->flag = VAR;
}

void DefinedVariable::setType(AttrNode *spec) {
    VariableType* type = (VariableType*)spec->baseNode;
    DefinedVariable* next = this;
    while (next!=nullptr){
        next->type = type;
        next = (DefinedVariable*)next->next;
    }
}

void DefinedVariable::addDimension(AttrNode* dim) {
    int newDim = stoi(dim->value);
    this->array.push_back(newDim);
}

DefinedFunction::DefinedFunction(AttrNode *functionID, AttrNode *paraList) {
    this->id = functionID->value;
    this->loc = new Location(functionID->lineNo, 0);
    this->flag = FUNC;
    parseParameters(paraList);
}

DefinedFunction::DefinedFunction(AttrNode *functionID) {
    this->id = functionID->value;
    this->loc = new Location(functionID->lineNo, 0);
    this->flag = FUNC;
}

void DefinedFunction::parseParameters(AttrNode* paraList) {
    DefinedVariable* var = (DefinedVariable*)paraList->baseNode;
    while (var!= nullptr){
        this->parameters.push_back(*var);
        var = (DefinedVariable*)var->next;
    }
}


list<DefinedVariable>& DefinedFunction::getParameters() {
    this->parameters;
}

AST *DefinedFunction::getBody() {
    return nullptr;
}

void DefinedFunction::setReturnType(AttrNode *type) {
    this->returnType = (VariableType*)type->baseNode;
}


Exp::Exp(AttrNode * terminal, DataType dataType) {
    this->loc = new Location(terminal->lineNo, 0);
    this->value = terminal->value;
    this->type = dataType;
}

BinaryExp::BinaryExp(AttrNode * le, AttrNode * rig, Operator operatorType) {
    this->left = (Exp*)le->baseNode;
    this->right = (Exp*)rig->baseNode;
    this->operatorType = operatorType;
    this->loc = left->loc;
}

SingleExp::SingleExp(AttrNode *operatedNode, Operator operatorType) {
    this->operated = (Exp*)operatedNode->baseNode;
    this->operatorType = operatorType;
    this->loc = operated->loc;
}

InvokeExp::InvokeExp(AttrNode *invoker) {
    this->functionName = invoker->value;
    this->loc = new Location(invoker->lineNo, 0);
    this->operatorType = Operator ::INVOKE;
}


void findEntity(Args* args, Exp* exp){
    Exp* next = exp;
    while (next != nullptr){
        args->args.push_back(*next);
        next = (Exp*)next->next;
    }
}

/**
 *
 * @param invoker
 * @param args its baseNode is link list of Exp
 */
InvokeExp::InvokeExp(AttrNode *invoker, AttrNode *args) {
    this->operatorType = Operator ::INVOKE;
    this->functionName = invoker->value;
    this->args = new Args;
    findEntity(this->args, (Exp*)args->baseNode);
    this->loc = this->args->loc;
}
