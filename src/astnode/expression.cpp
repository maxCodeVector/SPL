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
        return new Error{getLocation(), "not support continue now"};
    } else if (this->operatorType == BREAK_OP) {
        return new Error{getLocation(), "not support break now"};
    }
    if (this->type->getType() == REF_TYPE) {
        Entity *entity = scope->get(this->getValue());
        if (entity == nullptr) {
            return new Error{getLocation(), "can not found defined for:" + getValue()};
        }
        this->setReferenceVar(entity);
    }
    return nullptr;
}

bool Exp::isLeftValue() {
    return !(this->type->getType() == INT_TYPE
             || this->type->getType() == CHAR_TYPE
             || this->type->getType() == FLOAT_TYPE);
}

bool Exp::isArray(){
    if(this->type->getType() == INT_TYPE
        || this->type->getType() == CHAR_TYPE
        || this->type->getType() == FLOAT_TYPE
        || this->type->getType() == STRUCT_TYPE){
        return false;
    }
    DefinedVariable* var = (DefinedVariable*)this->referenceVar;
    if(var) {
        return var->isArray(this->dimension);
    }
    return true;
}

Exp::Exp(DataType dataType) {
    this->type = new VariableType(dataType);
}

VariableType *Exp::getType() {
    if(type->getType()==REF_TYPE && referenceVar!= nullptr)
        return ((DefinedVariable*)referenceVar)->getType()->getActualType();
    return type;
}


BinaryExp::BinaryExp(AttrNode *le, AttrNode *rig, Operator operatorType):Exp(DataType::INFER_TYPE) {
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

//Error *BinaryExp::inferType(){
//    if (this->operatorType == ASSIGN_OP) {
//        if (!left->isLeftValue())
//            return new Error{getLocation(), "can not assign to this type:" + getValue()};
//    }
//    //todo
//    if (left->getType() != right->getType()) {
//        return new Error{getLocation(),
//                         "can not resolve two different data type var:" + left->getName() + ", " + right->getValue()};
//
//    }
//    return nullptr;
//}

void BinaryExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    if (operatorType == ASSIGN_OP) {
        if (!left->isLeftValue()) {
            checker->error(new Error{getLocation(),
                                     "can not assign value to this type:" + left->getValue()});
            return;
        }
    }
    if (operatorType == ARRAY_INDEX_OP) {
        if(!left->isArray()){
            checker->error(new Error{getLocation(),
                                     "not a array:"+ left->getValue()});
        }
    }
}



Error *BinaryExp::inferType(ToplevelScope* toplevelScope) {
    Error* err = Exp::inferType(toplevelScope);
    if(err) {
        delete(err);
        err = nullptr;
        err = left->inferType(toplevelScope);
        if(err)
            return err;
        err = right->inferType(toplevelScope);
        if(err)
            return err;
        if (this->operatorType==ARRAY_INDEX_OP){
            if(!left->isArray()){
                return new Error{getLocation(), "non array but index:"+ right->getValue()};
            }
            if(right->getType()->getType()!=INT_TYPE){
                return new Error{getLocation(), "non integer index:"+ right->getValue()};
            }
            if(this->type->getType()==INFER_TYPE){
                delete(this->type);
                this->type = new VariableType(REF_TYPE);
            }
            this->setReferenceVar(left->getReferenceValue());
            this->indexOneDimension(this->left->getCurrentDimension());
            return nullptr;
        }
        if(this->operatorType==ASSIGN_OP){
//            return new Error{getLocation(), "assign expression do not have type"};
        }
        if(left->getType()->getType()!=right->getType()->getType())
            return new Error{getLocation(), "type need to same"};
        this->type = left->getType();
        return nullptr;
    }
    return nullptr;
}

UnaryExp::UnaryExp(AttrNode *operatedNode, Operator operatorType) :Exp(DataType::INFER_TYPE) {
    this->operand = (Exp *) operatedNode->baseNode;
    this->operatorType = operatorType;
    setLocation(operand->getLocation());
}

Error *UnaryExp::checkReference(Scope *scope) {
    return this->operand->checkReference(scope);
}

Error *UnaryExp::inferType(ToplevelScope* toplevelScope) {
    Error* err = Exp::inferType(nullptr);
    if(err) {
        delete(err);
        err = this->operand->inferType(nullptr);
        if(err) {
            return err;
        }
        if(operatorType==SUB_OP){

        }
        if(operatorType==NOT_OP) {
            if (operand->getType()->getType() != BOOL_TYPE) {
                return new Error{getLocation(), "apply not to non bool type"};
            }
        }
        return nullptr;
    }
    return nullptr;
}


InvokeExp::InvokeExp(AttrNode *invoker):Exp(DataType::INFER_TYPE) {
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
InvokeExp::InvokeExp(AttrNode *invoker, AttrNode *args) :Exp(DataType::INFER_TYPE) {
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


bool checkArgument(list<struct DefinedVariable *> list, Args *pArgs) {


    return true;
}

Error *InvokeExp::inferType(ToplevelScope* toplevelScope) {
    DefinedFunction* function = (DefinedFunction*)toplevelScope->get(this->functionName);
    VariableType* returnType = function->getReturnType();
    this->type = returnType;
    bool checkRes = checkArgument(function->getParameters(), this->args);
    if(!checkRes){
        return new Error{getLocation(), "function argument error"+functionName};
    }
    return nullptr;
}

GetAttributeExp::GetAttributeExp(AttrNode *operated, string &attributeName) :Exp(DataType::INFER_TYPE) {
    Exp *exp = (Exp *) operated->baseNode;
    this->attrName = attributeName;
    this->operatorType = DOT_OP;
    this->object = exp;
    this->setLocation(exp->getLocation());
}

void GetAttributeExp::acceptDereferenceCheck(DereferenceChecker *checker) {
    if (operatorType == DOT_OP) {
        if (getType()->getType() != STRUCT_TYPE) {
            checker->error(new Error{getLocation(),
                                     "can only get member from struct:" + getValue()});
            return;
        }
    }

}

Error *GetAttributeExp::inferType(ToplevelScope* toplevelScope) {
    Error* err = object->inferType(nullptr);
    if(err)
        return err;
    VariableType* father_type = object->getType();
    if(father_type->getType()!=STRUCT_TYPE){
        return new Error{getLocation(), "want to get attribute from none struct type var:"+this->attrName};
    }
    DefinedVariable* member = ((Struct*)father_type)->getMember(attrName);
    if(member== nullptr){
        return new Error{getLocation(), "want to visit a non existed member:"+attrName};
    }
    if(this->type->getType()==INFER_TYPE){
        delete(this->type);
        this->type = new VariableType(REF_TYPE);
    }
    setReferenceVar(member);
    return nullptr;
}

Error *GetAttributeExp::checkReference(Scope *scope) {
    this->object->checkReference(scope);
}

