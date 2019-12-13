//
// Created by hya on 11/20/19.
//

#include "expression.h"
#include "../semantic/scope.h"
#include "../semantic/type.h"
#include "../ir/irnode.h"

Exp::Exp(AttrNode *terminal, DataType dataType) {
    // only char, int, float and id(ref) could invoke this constructor
    setLocation(terminal->lineNo, 0);
    this->value = terminal->value;
    this->type = new VariableType(dataType);
    if (this->value == "break") {
        this->operatorType = BREAK_OP;
    } else if (this->value == "continue") {
        this->operatorType = CONT_OP;
    }
}

Error *Exp::checkReference(Scope *scope) {
    if (this->operatorType == CONT_OP) {
        return new Error{getLocation(), OTHER_ERROR, "not support continue now"};
    } else if (this->operatorType == BREAK_OP) {
        return new Error{getLocation(), OTHER_ERROR, "not support break now"};
    }
    // after doing this, all referenced elementType are connected to its corresponding variable definition
    if (this->type->getElementType() == REF_TYPE) {
        Entity *entity = scope->get(this->getValue());
        if (entity == nullptr) {
            return new Error{getLocation(), UNDEFINED_VAR, "can not found defined for:" + getValue()};
        }
        this->setReferenceVar(entity);
    }
    return nullptr;
}

bool Exp::isLeftValue() {
    return referenceVar != nullptr;
    return !(this->type->getElementType() == INT_TYPE
             || this->type->getElementType() == CHAR_TYPE
             || this->type->getElementType() == FLOAT_TYPE
             || this->type->getElementType() == DataType::BOOL_TYPE);
}

bool Exp::isArray() {
//    if (this->type->getElementType() == INT_TYPE
//        || this->type->getElementType() == CHAR_TYPE
//        || this->type->getElementType() == FLOAT_TYPE
//        || this->type->getElementType() == STRUCT_TYPE) {
//        return false;
//    }
//    Variable *var = (Variable *) this->referenceVar;
//    if (var) {
//        return var->isArray(this->dimension);
//    }
//    return true;
    return this->getType()->isArray();
}

Exp::Exp(DataType dataType) {
    this->type = new VariableType(dataType);
}

VariableType *Exp::getType() {
    if (type->getElementType() == REF_TYPE && referenceVar != nullptr) {
        VariableType *refType = ((Variable *) referenceVar)->getType()->getActualType();
        int currDim = this->dimension;
        if(currDim == 0)
            return refType;
        VariableType *inner_element = refType->getElement();
        while (currDim > 0) {
            if (inner_element == nullptr) {
                return nullptr;
            }
            inner_element = inner_element->getElement();
            currDim--;
        }
        return inner_element;
    }
    return type->getActualType();
}

Error *Exp::inferType(ToplevelScope *topLevel) {
    if (type->getElementType() == INFER_TYPE)
        return new Error{getLocation(), OTHER_ERROR, "elementType need to be inferred"};
    return nullptr;
}

bool Exp::isNumber() {
    return this->getType()->getElementType() == INT_TYPE || this->getType()->getElementType() == FLOAT_TYPE;
}

void Exp::accept(IRVisitor *visitor) {
    if (this->type->getElementType() == INT_TYPE) {
        setSymbol("#" + value);
    } else
        setSymbol(visitor->getAddress(value));
}

Operator Exp::getOperatorType() const {
    return operatorType;
}

const string &Exp::getSymbol() const {
    return symbol;
}

void Exp::setSymbol(const string &symbol) {
    Exp::symbol = symbol;
}

bool Exp::isPointer() const {
    return pointer;
}

void Exp::setPointer() {
    pointer = true;
}


BinaryExp::BinaryExp(AttrNode *le, AttrNode *rig, Operator operatorType) : Exp(DataType::INFER_TYPE) {
    this->left = (Exp *) le->baseNode;
    this->right = (Exp *) rig->baseNode;
    this->operatorType = operatorType;
    setLocation(left->getLocation());
}

Error *BinaryExp::checkReference(Scope *scope) {
    Error *error = this->left->checkReference(scope);
    if (error) {
        return error;
    }
    return this->right->checkReference(scope);
}

// until now, only check if assign to non-left va;ue
void BinaryExp::acceptDereferenceCheck(DereferenceChecker *checker) {
}


Error *BinaryExp::inferType(ToplevelScope *toplevelScope) {
    // avoid to enter recursive loop to infer elementType
    Error *err = Exp::inferType(toplevelScope);
    if (err) {
        delete (err);
        err = left->inferType(toplevelScope);
        if (err)
            return err;
        err = right->inferType(toplevelScope);
        if (err)
            return err;
        if (this->operatorType == ARRAY_INDEX_OP) {
            if (!left->isArray()) {
                return new Error{getLocation(), NON_ARRAY,
                                 "non array type but index:" + right->getValue()};
            }
            if (right->getType()->getElementType() != INT_TYPE) {
                return new Error{getLocation(), NON_INT_INDEX,
                                 "non integer index:" + right->getValue()};
            }
//            if (this->type->getElementType() == INFER_TYPE) {
//                delete (this->type);
//            }
            delete (this->type);
            this->type = left->getType()->getElement();
            this->setReferenceVar(left->getReferenceValue());
            this->indexOneDimension(this->left->getCurrentDimension());
            return nullptr;
        }

        if (this->operatorType == Operator::AND_OP
            || this->operatorType == Operator::OR_OP) {
            if (left->getType()->getElementType() != BOOL_TYPE || right->getType()->getElementType() != BOOL_TYPE) {
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "can not logic operate non-bool elementType:"};
            }
            free(this->type);
            this->type = new VariableType(BOOL_TYPE);
            return nullptr;
        }

        if (this->operatorType == EQ_OP
            || this->operatorType == Operator::LT_OP
            || this->operatorType == Operator::LE_OP
            || this->operatorType == Operator::GE_OP
            || this->operatorType == Operator::GT_OP
            || this->operatorType == Operator::NE_OP) {
            if (!left->isNumber() || !right->isNumber()) {
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "compare not support for this elementType expression"};
            }
            delete (this->type);
            this->type = new VariableType(BOOL_TYPE);
            return nullptr;
        }

        if (this->operatorType == ADD_OP
            || this->operatorType == Operator::SUB_OP
            || this->operatorType == Operator::MUL_OP
            || this->operatorType == Operator::DIV_OP) {
            if (!left->isNumber() || !right->isNumber()) {
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "arithmetic not support for this elementType expression"};
            }
            delete (this->type);
            if (left->getType()->getElementType() == FLOAT_TYPE
                || right->getType()->getElementType() == FLOAT_TYPE) {
                this->type = new VariableType(FLOAT_TYPE);
            } else
                this->type = new VariableType(left->getType()->getElementType());
            return nullptr;
        }

        if (operatorType == ASSIGN_OP) {
            if (!left->isLeftValue()) {
                return new Error{getLocation(), ASSIGN_LEFT_VALUE,
                                 "can not assign value to this elementType:" + left->getValue()};
            }
            if (!checkTypeEqualOfExp(left, right))
                return new Error{getLocation(), ASSIGN_DIFF_TYPE,
                                 "assign value between different elementType expression"};
            return nullptr;
        }

        if (!checkTypeEqualOfExp(left, right))
            return new Error{getLocation(), UNMATCHED_OPERATE,
                             "two expression elementType need to same"};
        delete (this->type);
        this->type = new VariableType(left->getType()->getElementType());
        return nullptr;
    }
    return nullptr;
}

void BinaryExp::accept(IRVisitor *visitor) {
    visitor->visit(this);
}

UnaryExp::UnaryExp(AttrNode *operatedNode, Operator operatorType) : Exp(DataType::INFER_TYPE) {
    this->operand = (Exp *) operatedNode->baseNode;
    this->operatorType = operatorType;
    setLocation(operand->getLocation());
}

Error *UnaryExp::checkReference(Scope *scope) {
    return this->operand->checkReference(scope);
}

Error *UnaryExp::inferType(ToplevelScope *toplevelScope) {
    Error *err = Exp::inferType(toplevelScope);
    if (err) {
        delete (err);
        err = this->operand->inferType(toplevelScope);
        if (err) {
            return err;
        }
        if (operatorType == SUB_OP) {
            DataType operand_type = operand->getType()->getElementType();
            if (!operand->isNumber())
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "can only solve negative of number:" + operand->getValue()};
        }
        if (operatorType == NOT_OP) {
            if (operand->getType()->getElementType() != BOOL_TYPE
//                || operand->getType()->getElementType() != INT_TYPE
                    ) {
                return new Error{getLocation(), UNMATCHED_OPERATE, "apply not operator to non-bool elementType"};
            }
        }
        this->type = operand->getType();
        return nullptr;
    }
    return nullptr;
}

void UnaryExp::accept(IRVisitor *visitor) {
    if(this->operand->type->getElementType()==INT_TYPE){
        if(this->operatorType==SUB_OP){
            setSymbol("#-"+operand->getValue());
            return;
        }
    }
    visitor->visit(this);
}


InvokeExp::InvokeExp(AttrNode *invoker) : Exp(DataType::INFER_TYPE) {
    this->functionName = invoker->value;
    setLocation(invoker->lineNo, 0);
    this->operatorType = Operator::INVOKE;
    this->args = new Args(getLocation());
}


void findEntity(Args *args, Exp *exp) {
    Exp *next = exp;
    while (next != nullptr) {
        args->args.push_back(next);
        next = (Exp *) next->next;
    }
}

/**
 *
 * @param invoker
 * @param args its baseNode is link list of Exp
 */
InvokeExp::InvokeExp(AttrNode *invoker, AttrNode *args) : Exp(DataType::INFER_TYPE) {
    this->operatorType = Operator::INVOKE;
    this->functionName = invoker->value;
    setLocation(invoker->lineNo, 0);
    this->args = new Args(getLocation());
    findEntity(this->args, (Exp *) args->baseNode);
}

Error *InvokeExp::checkReference(Scope *scope) {
    Entity *entity = scope->get(this->functionName);
    if (entity == nullptr) {
        return new Error{getLocation(), UNDEFINED_FUN, "not found this function:" + functionName};
    } else if (typeid(*entity) != typeid(Function)) {
        return new Error{getLocation(), NON_FUNC, "want to invoke non-function:" + functionName};
    }
    for (Exp *exp:args->args) {
        Error *error = exp->checkReference(scope);
        if (error)
            return error;
    }
    return nullptr;
}

void InvokeExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    for (Exp *arg: args->getArguments())
        arg->acceptDereferenceCheck(checker);
}

bool checkEqualVariableType(VariableType *type1, VariableType *type2);

bool checkStructEquivalence(Struct *st1, Struct *st2);


bool inline checkArraySize(VariableType *type1, VariableType *type2) {
    return type1->getArraySize() == type2->getArraySize();
}


bool checkEqualVariableType(VariableType *type1, VariableType *type2) {
    if (!checkArraySize(type1, type2))
        return false;
    // check element type if any
    if (type2->getElementType() != type1->getElementType()) {
        return false;
    }
    if (type1->getElementType() == STRUCT_TYPE) {
        if (type1->getTypeName() != type1->getTypeName()
            && !checkStructEquivalence((Struct *) type1, (Struct *) type2)) {
            return false;
        }
    }
    if (type1->getElementType() == ARRAY_TYPE) {
        return checkEqualVariableType(type1->getElement(), type2->getElement());
    }
    return true;
}


bool checkStructEquivalence(Struct *st1, Struct *st2) {
    list<Variable *> &members1 = st1->getMemberList();
    list<Variable *> &members2 = st2->getMemberList();
    if (members1.size() != members2.size()) {
        return false;
    }
    auto var1 = members1.begin();
    auto var2 = members2.begin();
    while (var1 != members1.end()) {
        if (!checkEqualVariableType((*var1)->getType(), (*var2)->getType())) {
            return false;
        }
        var1++;
        var2++;
    }
    return true;
}


bool checkTypeEqualOfExp(Exp *exp1, Exp *exp2) {
    return checkEqualVariableType(exp1->getType(), exp2->getType());
//    Variable *var1 = exp1->getReferenceValue();
//    int dim1 = exp1->getCurrentDimension();
//    Variable *var2 = exp2->getReferenceValue();
//    int dim2 = exp2->getCurrentDimension();
//    if (var1 != nullptr && var2 != nullptr) {
//        if (var1->getType()->getElementType() != var2->getType()->getElementType()) {
//            return false;
//        } else if (var1->getType()->getElementType() == STRUCT_TYPE) {
//            if (var1->getType()->getTypeName() != var1->getType()->getTypeName()
//                || !checkStructEquivalence((Struct *) var1->getType(), (Struct *) var2->getType())) {
//                return false;
//            }
//        }
//        return checkArraySize(var1, var2, dim1, dim2);
//    }
//    if (var1 == nullptr) {
//        if (exp1->getType()->getElementType() != exp2->getType()->getElementType()) {
//            return false;
//        }
//        return !exp2->isArray();
//    }
//    if (exp1->getType()->getElementType() != exp2->getType()->getElementType()) {
//        return false;
//    }
//    return !exp1->isArray();
}


bool inline CheckEqualVariableAndExp(Variable *var, Exp *exp) {
    return checkEqualVariableType(var->getType(), exp->getType());
//    Variable *expRefVar = exp->getReferenceValue();
//    int currDimension = exp->getCurrentDimension();
//    if (expRefVar == nullptr) {
//        // it may be a value
//        if (var->isArray(0))// a value can not be a temp_array_size
//            return false;
//        if (var->getType()->getElementType() == FLOAT_TYPE) {
//            if (exp->getType()->getElementType() != INT_TYPE
//                && exp->getType()->getElementType() != FLOAT_TYPE)
//                return false;
//        } else if (exp->getType()->getElementType() != var->getType()->getElementType()) {
//            return false;
//        }
//    } else {
//        if (var->getType()->getElementType() == FLOAT_TYPE) {
//            if (exp->getType()->getElementType() != INT_TYPE
//                && exp->getType()->getElementType() != FLOAT_TYPE)
//                return false;
//        } else if (expRefVar->getType()->getElementType() != var->getType()->getElementType()) {
//            return false;
//        } else if (expRefVar->getType()->getElementType() == STRUCT_TYPE) {
//            if (var->getType()->getTypeName() != expRefVar->getType()->getTypeName()
//                || !checkStructEquivalence((Struct *) var->getType(), (Struct *) expRefVar->getType())) {
//                return false;
//            }
//        }
//        // temp_array_size check
//        if (!checkArraySize(var, expRefVar, 0, currDimension))
//            return false;
//    }
//    return true;
}

Error *checkArgument(list<struct Variable *> paras, Args *pArgs) {
    list<Exp *> &args = pArgs->getArguments();
    if (paras.size() != args.size())
        return new Error{pArgs->getLocation(), MIS_ARGUMENT, "argument elementType number not match:"};
    auto paraItor = paras.begin();
    auto argItor = args.begin();
    while (paraItor != paras.end()) {
        bool equal = CheckEqualVariableAndExp(*paraItor, *argItor);
        if (!equal)
            return new Error{pArgs->getLocation(), MIS_ARGUMENT, "argument elementType not match:"};
        argItor++;
        paraItor++;
    }
    return nullptr;
}

Error *InvokeExp::inferType(ToplevelScope *toplevelScope) {
    for (Exp *arg: args->getArguments()) {
        arg->inferType(toplevelScope);
    }
    Function *function = (Function *) toplevelScope->get(this->functionName);
    VariableType *returnType = function->getReturnType();
    this->type = returnType;
    return checkArgument(function->getParameters(), this->args);
}

void InvokeExp::accept(IRVisitor *visitor) {
    visitor->visit(this);
}

GetAttributeExp::GetAttributeExp(AttrNode *operated, string &attributeName) : Exp(DataType::INFER_TYPE) {
    Exp *exp = (Exp *) operated->baseNode;
    this->attrName = attributeName;
    this->operatorType = DOT_OP;
    this->object = exp;
    this->setLocation(exp->getLocation());
}

void GetAttributeExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    if (operatorType == DOT_OP) {
        if (getType()->getElementType() != STRUCT_TYPE) {
            checker->error(new Error{getLocation(), NON_STRUCT,
                                     "can only get member from struct:" + getValue()});
            return;
        }
    }

}

Error *GetAttributeExp::inferType(ToplevelScope *toplevelScope) {
    Error *err = object->inferType(toplevelScope);
    if (err)
        return err;
    VariableType *father_type = object->getType()->getActualType();
    if (father_type->getElementType() != STRUCT_TYPE) {
        return new Error{getLocation(), NON_STRUCT,
                         "want to get attribute from none struct elementType var:" + this->attrName};
    }
    Variable *member = ((Struct *) father_type)->getMember(attrName);
    if (member == nullptr) {
        return new Error{getLocation(), UNDEFINED_MEMBER, "want to visit a non existed member:" + attrName};
    }
    if (this->type->getElementType() == INFER_TYPE) {
        delete (this->type);
        this->type = new VariableType(REF_TYPE);
    }
    setReferenceVar(member);
    return nullptr;
}

Error *GetAttributeExp::checkReference(Scope *scope) {
    return this->object->checkReference(scope);
}

void GetAttributeExp::accept(IRVisitor *visitor) {
    visitor->visit(this);
}

const string &GetAttributeExp::getAttrName() const {
    return attrName;
}

