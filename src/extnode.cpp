#include "extnode.h"
#include "scope.hpp"


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
    while (next!=nullptr){
        next->type = new VariableType(*type);
        next = (DefinedVariable*)next->next;
    }
    delete(type);
}

void DefinedVariable::addDimension(AttrNode* dim) {
    int newDim = stoi(dim->value);
    this->array.push_back(newDim);
}

DefinedVariable::~DefinedVariable() {
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


Exp::Exp(AttrNode * terminal, DataType dataType) {
    setLocation(new Location(terminal->lineNo, 0));
    this->value = terminal->value;
    this->type = dataType;
    if(this->value=="break"){
        this->operatorType = BREAK_OP;
    } else if(this->value=="continue"){
        this->operatorType = CONT_OP;
    }
}

Error * Exp::checkReference(Scope *scope) {
    if(this->operatorType==CONT_OP){
        return new Error{getLocation(), "not support continue now"};
    } else if(this->operatorType==BREAK_OP){
        return new Error{getLocation(), "not support break now"};
    }
    if(this->type==OTHER_TYPE){
        Entity* entity = scope->get(this->getName());
        if(entity== nullptr){
            return new Error{getLocation(), "can not found defined for:" + getName()};
        }
        this->setReferenceVar(entity);
    }
    return nullptr;
}

BinaryExp::BinaryExp(AttrNode * le, AttrNode * rig, Operator operatorType) {
    this->left = (Exp*)le->baseNode;
    this->right = (Exp*)rig->baseNode;
    this->operatorType = operatorType;
    setLocation(left->getLocation());
}

Error * BinaryExp::checkReference(Scope *scope) {
    Error* error = this->left->checkReference(scope);
    if(error){
        return error;
    }
    return this->right->checkReference(scope);
}

UnaryExp::UnaryExp(AttrNode *operatedNode, Operator operatorType) {
    this->operand = (Exp*)operatedNode->baseNode;
    this->operatorType = operatorType;
    setLocation(operand->getLocation());
}

Error *UnaryExp::checkReference(Scope *scope) {
    return this->operand->checkReference(scope);
}


InvokeExp::InvokeExp(AttrNode *invoker) {
    this->functionName = invoker->value;
    setLocation(new Location(invoker->lineNo, 0));
    this->operatorType = Operator ::INVOKE;
    this->args = new Args;
}


void findEntity(Args* args, Exp* exp){
    Exp* next = exp;
    while (next != nullptr){
        args->args.push_back(next);
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
    setLocation(invoker->lineNo, 0);
}

Error *InvokeExp::checkReference(Scope *scope) {
    Entity* entity = scope->get(this->functionName);
    if(entity== nullptr){
        return new Error{getLocation(), "not found this function:" + functionName};
    } else if(entity->flag!=FUNC){
        return new Error{getLocation(), "want to invoke non-function:" + functionName};
    }
    for(Exp* exp:args->args) {
        Error* error = exp->checkReference(scope);
        if(error)
            return error;
    }
    return nullptr;
}


Body::Body(AttrNode *defList, AttrNode *stmtList) {
    DefinedVariable* variable = (DefinedVariable*)defList->baseNode;
    while (variable!= nullptr){
        this->vars.push_back(variable);
        variable = (DefinedVariable*)variable->next;
    }
    Statement* statement = (Statement*)stmtList->baseNode;
    while (statement!= nullptr){
        this->statements.push_back(statement);
        statement = (Statement*)statement->next;
    }
}

Statement::Statement(AttrNode *exp) {
    this->exp = (Exp*)exp->baseNode;
}

Struct::~Struct() {
    free_all(members);
}

Struct::Struct(AttrNode *name):VariableType(STRUCT_TYPE){
    this->typeName = name->value;
}

Struct::Struct(AttrNode *name, AttrNode *defList):VariableType(STRUCT_TYPE){
    this->typeName = name->value;
    DefinedVariable* nextVar = (DefinedVariable*)defList->baseNode;
    while (nextVar!= nullptr){
        this->members.push_back(nextVar);
        nextVar = (DefinedVariable*)nextVar->next;
    }
}

DeclaredTypeVariable::DeclaredTypeVariable(AttrNode *spec){
    this->flag = DECLARATION;
    VariableType* variableType = (VariableType*)spec->baseNode;
    this->type = variableType;
}

GetAttributeExp::GetAttributeExp(AttrNode *operated, string& attributeName) {
    Exp* exp = (Exp*)operated->baseNode;
    this->attrName = attributeName;
    this->operatorType = DOT_OP;
    this->object = exp;
}
