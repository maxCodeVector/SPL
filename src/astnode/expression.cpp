//
// Created by hya on 11/20/19.
//

#include "expression.h"
#include "../scope.h"
#include "../type.h"

Exp::Exp(AttrNode *terminal, DataType dataType) {
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
        return new Error{getLocation(), "not support continue now"};
    } else if (this->operatorType == BREAK_OP) {
        return new Error{getLocation(), "not support break now"};
    }
    if (this->type->getType() == REF_TYPE) {
        Entity *entity = scope->get(this->getName());
        if (entity == nullptr) {
            return new Error{getLocation(), "can not found defined for:" + getName()};
        }
        this->setReferenceVar(entity);
    }
    return nullptr;
}

bool Exp::isLeftValue() {
    return this->type->getType() == DataType::REF_TYPE;
}

bool Exp::isArray() {
    return this->type->isArray(dimension);
}


BinaryExp::BinaryExp(AttrNode *le, AttrNode *rig, Operator operatorType) {
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

Error *BinaryExp::checkType() {
    if (this->operatorType == ASSIGN_OP) {
        if (!left->isLeftValue())
            return new Error{getLocation(), "can not assign to this type:" + getName()};
    }
    //todo
    if (left->getType() != right->getType()) {
        return new Error{getLocation(),
                         "can not resolve two different data type var:" + left->getName() + ", " + right->getName()};

    }
    return nullptr;
}

void BinaryExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    if (operatorType == ASSIGN_OP) {
        if (!left->isLeftValue()) {
            checker->error(new Error{getLocation(),
                                     "can not assign value to this type:" + left->getName()});
            return;
        }
    }
    if (operatorType == ARRAY_INDEX_OP) {
        if(left->isArray()){
            checker->error(new Error{getLocation(),
                                     "not a array"+left->getName()});
        }
    }
}

UnaryExp::UnaryExp(AttrNode *operatedNode, Operator operatorType) {
    this->operand = (Exp *) operatedNode->baseNode;
    this->operatorType = operatorType;
    setLocation(operand->getLocation());
}

Error *UnaryExp::checkReference(Scope *scope) {
    return this->operand->checkReference(scope);
}


InvokeExp::InvokeExp(AttrNode *invoker) {
    this->functionName = invoker->value;
    setLocation(new Location(invoker->lineNo, 0));
    this->operatorType = Operator::INVOKE;
    this->args = new Args;
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
InvokeExp::InvokeExp(AttrNode *invoker, AttrNode *args) {
    this->operatorType = Operator::INVOKE;
    this->functionName = invoker->value;
    this->args = new Args;
    findEntity(this->args, (Exp *) args->baseNode);
    setLocation(invoker->lineNo, 0);
}

Error *InvokeExp::checkReference(Scope *scope) {
    Entity *entity = scope->get(this->functionName);
    if (entity == nullptr) {
        return new Error{getLocation(), "not found this function:" + functionName};
    } else if (entity->flag != FUNC) {
        return new Error{getLocation(), "want to invoke non-function:" + functionName};
    }
    for (Exp *exp:args->args) {
        Error *error = exp->checkReference(scope);
        if (error)
            return error;
    }
    return nullptr;
}

void InvokeExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    Exp::acceptDereferenceCheck(checker);
}

GetAttributeExp::GetAttributeExp(AttrNode *operated, string &attributeName) {
    Exp *exp = (Exp *) operated->baseNode;
    this->attrName = attributeName;
    this->operatorType = DOT_OP;
    this->object = exp;
    this->setLocation(exp->getLocation());
    this->type = new VariableType(REF_TYPE);
}

void GetAttributeExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    if (operatorType == DOT_OP) {
        if (getType()->getType() != STRUCT_TYPE) {
            checker->error(new Error{getLocation(),
                                     "can only get member from struct:" + getName()});
            return;
        }
    }

}

