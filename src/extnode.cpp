#include "extnode.hpp"
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
    this->loc = new Location(varDec->lineNo, 0);
    this->loc_alloc = true;
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
    this->loc = new Location(functionID->lineNo, 0);
    this->loc_alloc = true;
    this->flag = FUNC;
    parseParameters(paraList);
}

DefinedFunction::DefinedFunction(AttrNode *functionID) {
    this->id = functionID->value;
    this->loc = new Location(functionID->lineNo, 0);
    this->loc_alloc = true;
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
    this->loc = new Location(terminal->lineNo, 0);
    this->loc_alloc = true;
    this->value = terminal->value;
    this->type = dataType;
}

Error * Exp::checkReference(Scope *scope) {
    if(this->type==OTHER_TYPE){
        Entity* entity = scope->get(this->getName());
        if(entity== nullptr){
            return new Error{location(), "can not found defined for:"+getName()};
        }
        this->setReferenceVar(entity);
    }
    return nullptr;
}

BinaryExp::BinaryExp(AttrNode * le, AttrNode * rig, Operator operatorType) {
    this->left = (Exp*)le->baseNode;
    this->right = (Exp*)rig->baseNode;
    this->operatorType = operatorType;
    this->loc = left->location();
}

Error * BinaryExp::checkReference(Scope *scope) {
    Error* error = this->left->checkReference(scope);
    if(error){
        return error;
    }
    return this->right->checkReference(scope);
}

SingleExp::SingleExp(AttrNode *operatedNode, Operator operatorType) {
    this->operated = (Exp*)operatedNode->baseNode;
    this->operatorType = operatorType;
    this->loc = operated->location();
}

Error *SingleExp::checkReference(Scope *scope) {
    return this->operated->checkReference(scope);
}

InvokeExp::InvokeExp(AttrNode *invoker) {
    this->functionName = invoker->value;
    this->loc = new Location(invoker->lineNo, 0);
    this->loc_alloc = true;
    this->operatorType = Operator ::INVOKE;
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
    this->loc = this->args->location();
}

Error *InvokeExp::checkReference(Scope *scope) {
    for(Exp* exp:args->args) {
        return exp->checkReference(scope);
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

