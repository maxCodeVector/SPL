//
// Created by hya on 11/20/19.
//

#include "expression.h"
#include "../scope.h"
#include "../type.h"

Exp::Exp(AttrNode *terminal, DataType dataType) {
    // only char, int, float and id(ref) could invoke this constructor
    setLocation(new Location(terminal->lineNo, 0));
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
    // after doing this, all referenced type are connected to its corresponding variable definition
    if (this->type->getType() == REF_TYPE) {
        Entity *entity = scope->get(this->getValue());
        if (entity == nullptr) {
            return new Error{getLocation(), UNDEFINED_VAR, "can not found defined for:" + getValue()};
        }
        this->setReferenceVar(entity);
    }
    return nullptr;
}

bool Exp::isLeftValue() {
    return !(this->type->getType() == INT_TYPE
             || this->type->getType() == CHAR_TYPE
             || this->type->getType() == FLOAT_TYPE
             || this->type->getType() == DataType::BOOL_TYPE);
}

bool Exp::isArray() {
    if (this->type->getType() == INT_TYPE
        || this->type->getType() == CHAR_TYPE
        || this->type->getType() == FLOAT_TYPE
        || this->type->getType() == STRUCT_TYPE) {
        return false;
    }
    DefinedVariable *var = (DefinedVariable *) this->referenceVar;
    if (var) {
        return var->isArray(this->dimension);
    }
    return true;
}

Exp::Exp(DataType dataType) {
    this->type = new VariableType(dataType);
}

VariableType *Exp::getType() {
    if (type->getType() == REF_TYPE && referenceVar != nullptr)
        return ((DefinedVariable *) referenceVar)->getType()->getActualType();
    return type;
}

Error *Exp::inferType(ToplevelScope *topLevel) {
    if (type->getType() == INFER_TYPE)
        return new Error{getLocation(), OTHER_ERROR, "type need to be inferred"};
    return nullptr;
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
    // avoid to enter recursive loop to infer type
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
                                 "non array but index:" + right->getValue()};
            }
            if (right->getType()->getType() != INT_TYPE) {
                return new Error{getLocation(), NON_INT_INDEX,
                                 "non integer index:" + right->getValue()};
            }
            if (this->type->getType() == INFER_TYPE) {
                delete (this->type);
                this->type = new VariableType(REF_TYPE);
            }
            this->setReferenceVar(left->getReferenceValue());
            this->indexOneDimension(this->left->getCurrentDimension());
            return nullptr;
        }

        if (this->operatorType == Operator::AND_OP
            || this->operatorType == Operator::OR_OP) {
            if (left->getType()->getType() != BOOL_TYPE || right->getType()->getType() != BOOL_TYPE) {
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "can not logic operate non-bool type:"};
            }
            this->type = new VariableType(BOOL_TYPE);
            return nullptr;
        }

        if (this->operatorType == EQ_OP
            || this->operatorType == Operator::LT_OP
            || this->operatorType == Operator::LE_OP
            || this->operatorType == Operator::GE_OP
            || this->operatorType == Operator::GT_OP
            || this->operatorType == Operator::NE_OP) {
            if ((left->getType()->getType() != INT_TYPE
                 && left->getType()->getType() != FLOAT_TYPE)
                || (right->getType()->getType() != INT_TYPE
                    && right->getType()->getType() != FLOAT_TYPE)
                    ) {
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "compare not support for this type expression"};
            }
            this->type = new VariableType(BOOL_TYPE);
            return nullptr;
        }

        if (this->operatorType == ADD_OP
            || this->operatorType == Operator::SUB_OP
            || this->operatorType == Operator::MUL_OP
            || this->operatorType == Operator::DIV_OP) {
            if ((left->getType()->getType() != INT_TYPE
                 && left->getType()->getType() != FLOAT_TYPE)
                || (right->getType()->getType() != INT_TYPE
                    && right->getType()->getType() != FLOAT_TYPE)
                    ) {
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "arithmetic not support for this type expression"};
            }
            if (left->getType()->getType() == FLOAT_TYPE
                || right->getType()->getType() == FLOAT_TYPE) {
                this->type = new VariableType(FLOAT_TYPE);
            }
            this->type = left->getType();
            return nullptr;
        }

        if (operatorType == ASSIGN_OP) {
            if (!left->isLeftValue()) {
                return new Error{getLocation(), ASSIGN_LEFT_VALUE,
                                 "can not assign value to this type:" + left->getValue()};
            }
            if (!checkEqualExp(left, right))
                return new Error{getLocation(), ASSIGN_DIFF_TYPE,
                                 "assign value between different type expression"};
        }

        if (!checkEqualExp(left, right))
            return new Error{getLocation(), UNMATCHED_OPERATE, "two expression type need to same"};
        this->type = left->getType();
        return nullptr;
    }
    return nullptr;
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
            DataType operand_type = operand->getType()->getType();
            if (operand_type != INT_TYPE && operand_type != FLOAT_TYPE)
                return new Error{getLocation(), UNMATCHED_OPERATE,
                                 "can only solve negative of number:" + operand->getValue()};
        }
        if (operatorType == NOT_OP) {
            if (operand->getType()->getType() != BOOL_TYPE
//                || operand->getType()->getType() != INT_TYPE
                    ) {
                return new Error{getLocation(), UNMATCHED_OPERATE, "apply not operator to non-bool type"};
            }
        }
        this->type = operand->getType();
        return nullptr;
    }
    return nullptr;
}


InvokeExp::InvokeExp(AttrNode *invoker) : Exp(DataType::INFER_TYPE) {
    this->functionName = invoker->value;
    setLocation(new Location(invoker->lineNo, 0));
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
    } else if (entity->flag != FUNC) {
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


bool checkArray(DefinedVariable *var1, DefinedVariable *var2, int dim1, int dim2) {
    list<int> var1Arr = var1->getArrayDimension();
    list<int> var2Arr = var2->getArrayDimension();
    while (!var1Arr.empty()) {
        if (dim1 <= 0) {
            break;
        }
        dim1--;
        var1Arr.pop_front();
    }
    while (!var2Arr.empty()) {
        if (dim2 <= 0) {
            break;
        }
        dim2--;
        var2Arr.pop_front();
    }
    if (var1Arr.size() != var2Arr.size())
        return false;
    auto varItor1 = var1Arr.begin();
    auto varItor2 = var2Arr.begin();
    while (varItor1 != var1Arr.end()) {
        if (*varItor1 != *varItor2) {
            return false;
        }
        varItor1++;
        varItor2++;
    }
    return true;
}

bool checkStructEquivalence(Struct *st1, Struct *st2);

bool checkEqualVariable(DefinedVariable *var1, DefinedVariable *var2) {
    if (var2->getType()->getType() != var1->getType()->getType()) {
        return false;
    } else if (var2->getType()->getType() == STRUCT_TYPE) {
        if (var1->getType()->getTypeName() != var1->getType()->getTypeName()
            || !checkStructEquivalence((Struct *) var1->getType(), (Struct *) var2->getType())) {
            return false;
        }
    }
    // array check
    return checkArray(var1, var2, 0, 0);
}


bool checkStructEquivalence(Struct *st1, Struct *st2) {
    list<DefinedVariable *>& members1 = st1->getMemberList();
    list<DefinedVariable *>& members2 = st2->getMemberList();
    if (members1.size() != members2.size()) {
        return false;
    }
    auto var1 = members1.begin();
    auto var2 = members2.begin();
    while (var1 != members1.end()) {
        if (!checkEqualVariable(*var1, *var2)) {
            return false;
        }
        var1++;
        var2++;
    }
    return true;
}


bool checkEqualExp(Exp *exp1, Exp *exp2) {
    DefinedVariable *var1 = exp1->getReferenceValue();
    int dim1 = exp1->getCurrentDimension();
    DefinedVariable *var2 = exp2->getReferenceValue();
    int dim2 = exp2->getCurrentDimension();
    if (var1 != nullptr && var2 != nullptr) {
        if (var1->getType()->getType() != var2->getType()->getType()) {
            return false;
        } else if (var1->getType()->getType() == STRUCT_TYPE) {
            if (var1->getType()->getTypeName() != var1->getType()->getTypeName()
                || !checkStructEquivalence((Struct *) var1->getType(), (Struct *) var2->getType())) {
                return false;
            }
        }
        return checkArray(var1, var2, dim1, dim2);
    }
    if (var1 == nullptr) {
        if (exp1->getType()->getType() != exp2->getType()->getType()) {
            return false;
        }
        return !exp2->isArray();
    }
    if (exp1->getType()->getType() != exp2->getType()->getType()) {
        return false;
    }
    return !exp1->isArray();
}


bool CheckEqualVariableAndExp(DefinedVariable *var, Exp *exp) {
    DefinedVariable *expRefVar = exp->getReferenceValue();
    int currDimension = exp->getCurrentDimension();
    if (expRefVar == nullptr) {
        // it may be a value
        if (var->isArray(0))// a value can not be a array
            return false;
        if (var->getType()->getType() != exp->getType()->getType())
            return false;
    } else {
        if (expRefVar->getType()->getType() != var->getType()->getType()) {
            return false;
        } else if (expRefVar->getType()->getType() == STRUCT_TYPE) {
            if (var->getType()->getTypeName() != expRefVar->getType()->getTypeName()
                || !checkStructEquivalence((Struct *) var->getType(), (Struct *) expRefVar->getType())) {
                return false;
            }
        }
        // array check
        if (!checkArray(var, expRefVar, 0, currDimension))
            return false;
    }
    return true;
}

Error *checkArgument(list<struct DefinedVariable *> paras, Args *pArgs) {
    list<Exp *> &args = pArgs->getArguments();
    if (paras.size() != args.size())
        return new Error{pArgs->getLocation(), MIS_ARGUMENT, "argument type number not match:"};
    auto paraItor = paras.begin();
    auto argItor = args.begin();
    while (paraItor != paras.end()) {
        bool equal = CheckEqualVariableAndExp(*paraItor, *argItor);
        if (!equal)
            return new Error{pArgs->getLocation(), MIS_ARGUMENT, "argument type not match:"};
        argItor++;
        paraItor++;
    }
    return nullptr;
}

Error *InvokeExp::inferType(ToplevelScope *toplevelScope) {
    for (Exp *arg: args->getArguments()) {
        arg->inferType(toplevelScope);
    }
    DefinedFunction *function = (DefinedFunction *) toplevelScope->get(this->functionName);
    VariableType *returnType = function->getReturnType();
    this->type = returnType;
    return checkArgument(function->getParameters(), this->args);
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
        if (getType()->getType() != STRUCT_TYPE) {
            checker->error(new Error{getLocation(), NON_STRUCT,
                                     "can only get member from struct:" + getValue()});
            return;
        }
    }

}

Error *GetAttributeExp::inferType(ToplevelScope *toplevelScope) {
    Error *err = object->inferType(nullptr);
    if (err)
        return err;
    VariableType *father_type = object->getType();
    if (father_type->getType() != STRUCT_TYPE) {
        return new Error{getLocation(), NON_STRUCT,
                         "want to get attribute from none struct type var:" + this->attrName};
    }
    DefinedVariable *member = ((Struct *) father_type)->getMember(attrName);
    if (member == nullptr) {
        return new Error{getLocation(), UNDEFINED_MEMBER, "want to visit a non existed member:" + attrName};
    }
    if (this->type->getType() == INFER_TYPE) {
        delete (this->type);
        this->type = new VariableType(REF_TYPE);
    }
    setReferenceVar(member);
    return nullptr;
}

Error *GetAttributeExp::checkReference(Scope *scope) {
    return this->object->checkReference(scope);
}

