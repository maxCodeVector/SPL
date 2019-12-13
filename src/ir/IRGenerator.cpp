//
// Created by hya on 12/8/19.
//

#include "IRGenerator.h"
#include "optimizer.h"

TempNameGenerator::TempNameGenerator(const string &prefix, int size) {
    this->prefix = prefix;
    if (size < 0) {// means size is infinite
        this->allocated = nullptr;
        this->curr_max_id_num = 0;
    } else {
        this->allocated = new int[size]{0};
        this->curr_max_id_num = size;
    }
}

int TempNameGenerator::allocate() {
    if (!allocated) {
        return curr_max_id_num++;
    }
    for (int i = 0; i < curr_max_id_num; i++) {
        if (allocated[i] == 0) {
            allocated[i] = 1;
            return i;
        }
    }
    return -1;
}

TempNameGenerator::~TempNameGenerator() {
    delete[] allocated;
}

void TempNameGenerator::release(int numberId) {
    if (allocated && numberId < curr_max_id_num) {
        allocated[numberId] = 0;
    }
}

string TempNameGenerator::generateName(int numId) {
    return prefix + to_string(numId);
}

void TempNameGenerator::releaseAll(bool strong = false) {
    if (allocated) {
        for (int i = 0; i < curr_max_id_num; i++) {
            allocated[i] = 0;
        }
    } else if (strong) {
        this->curr_max_id_num = 0;
    }
}


IRGenerator::IRGenerator() {
    this->label = new TempNameGenerator("label", -1);
    this->tempVariable = new TempNameGenerator("t", -1);
    this->pointer = new TempNameGenerator("p", -1);
    this->varVariable = new TempNameGenerator("v", -1);
    arithmeticMap.insert(pair<Operator, IROperator>(ADD_OP, IR_ADD));
    arithmeticMap.insert(pair<Operator, IROperator>(SUB_OP, IR_SUB));
    arithmeticMap.insert(pair<Operator, IROperator>(MUL_OP, IR_MUL));
    arithmeticMap.insert(pair<Operator, IROperator>(DIV_OP, IR_DIV));

    compareOppositeMap.insert(pair<Operator, IROperator>(LT_OP, IR_IF_GE));
    compareOppositeMap.insert(pair<Operator, IROperator>(LE_OP, IR_IF_GT));
    compareOppositeMap.insert(pair<Operator, IROperator>(GT_OP, IR_IF_LE));
    compareOppositeMap.insert(pair<Operator, IROperator>(GE_OP, IR_IF_LT));
    compareOppositeMap.insert(pair<Operator, IROperator>(NE_OP, IR_IF_EQ));
    compareOppositeMap.insert(pair<Operator, IROperator>(EQ_OP, IR_IF_NE));
}

void IRGenerator::transformStmt(Statement *statement) {
    statement->accept(this);
}

IRExpr *IRGenerator::transformExpr(Exp *exp) {
    exp->accept(this);
    return nullptr;
}

IR *IRGenerator::generate(AST &ast) {
    currAst = &ast;
    for (Variable *variable: ast.getDefinedVars()) {
        if (variable->hasInitializer()) {
            variable->setIR(transformExpr(variable->getInitializer()));
        }
    }
    for (Function *function:ast.getFunctions()) {
        initCurrScopeSymbol((LocalScope *) function->getScope());
        Optimizer irOptimizer;
        IRStatement *insts = complileFunctionBody(function);
        if (insts) {
            cerr << "before optimized, inst number:" << insts->getInstructions()->size() << endl;
            checkJumpLinks(insts);
            function->setIr(irOptimizer.optimize(insts));
            cerr << "after optimized, inst number:" << insts->getInstructions()->size() << endl;
        }
    }
    if (this->errorHandler.errorOccured()) {
        errorHandler.showError(cerr);
        return nullptr;
    }
    return ast.getIR();
}

IRStatement *IRGenerator::complileFunctionBody(Function *f) {
    if (f->flag == BUILD_NODE) {
        return nullptr;
    }
    this->currIrStatement = new IRStatement;
    currIrStatement->addInstruction(IROperator::IR_FUNCTION, f->getName());
    for (Variable *para: f->getParameters()) {
        currIrStatement->addInstruction(IR_PARAM, getAddress(para->getName()));
    }
    this->jumpMap.clear();
    transformStmt(f->getBody());
    return currIrStatement;
}

void IRGenerator::visit(Variable *variable) {
    if (variable->hasInitializer()) {
        Exp *value = variable->getInitializer();
        value->accept(this);
        currIrStatement->addInstruction(IR_ASSIGN, getAddress(variable->getName()), value->getSymbol(), "");
//        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        tempVariable->releaseAll();
    }
    if (variable->getType()->getElementType() == STRUCT_TYPE || variable->getType()->isArray()) {
        string object = pointer->generateName(pointer->allocate());
        int storageSize = variable->getType()->getSize();
        currIrStatement->addInstruction(IR_DEC, object, to_string(storageSize), "");
        currIrStatement->addInstruction(IR_ADDRESS, getAddress(variable->getName()), object, "");
        return;
    }
}

void IRGenerator::visit(BinaryExp *expNode) {
    expNode->left->accept(this);
    expNode->right->accept(this);
    if (expNode->getOperatorType() == ASSIGN_OP) {
        // what about both of them are pointer
        if (expNode->left->isPointer() && expNode->right->isPointer()) {
            string tempValue = tempVariable->generateName(tempVariable->allocate());
            currIrStatement->addInstruction(IR_GET_VALUE_IN_ADDRESS, tempValue,
                                            expNode->right->getSymbol(), "");
            currIrStatement->addInstruction(IR_COPY_VALUE_TO_ADDRESS, expNode->left->getSymbol(),
                                            tempValue, "");
        } else if (expNode->left->isPointer()) {
            currIrStatement->addInstruction(IR_COPY_VALUE_TO_ADDRESS, expNode->left->getSymbol(),
                                            expNode->right->getSymbol(), "");
        } else if (expNode->right->isPointer()) {
            currIrStatement->addInstruction(IR_GET_VALUE_IN_ADDRESS, expNode->left->getSymbol(),
                                            expNode->right->getSymbol(), "");
        } else {
            currIrStatement->addInstruction(IR_ASSIGN, expNode->left->getSymbol(), expNode->right->getSymbol(), "");
//        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        }
        tempVariable->releaseAll();
        return;
    }
    auto item = arithmeticMap.find(expNode->getOperatorType());
    if (item != arithmeticMap.end()) {
        string leftSymbol, rightSymbol;
        getValueInBinaryExp(expNode->left, expNode->right, &leftSymbol, &rightSymbol);

        auto inst = new IRInst(item->second, "tt", leftSymbol, rightSymbol);
        int flag;
        // flag be -1 means only optimized half (one argument), not calculate the real value
        if (Optimizer::cacExpression(inst, &flag)) {
            switch (flag) {
                case 1:
                    expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
                    inst->target = expNode->getSymbol();
                    currIrStatement->addInstruction(inst);
                    break;
                case 2:
                    expNode->setSymbol(inst->arg1);
                    delete (inst);
                    break;
                case -1:
                    this->errorHandler.recordError(expNode->getLocation(), ErrorType::OTHER_ERROR, "divide by 0");
                    expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
                    inst->target = expNode->getSymbol();
                    currIrStatement->addInstruction(inst);
                default:
                    break;
            }
        } else {
            expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
            inst->target = expNode->getSymbol();
            currIrStatement->addInstruction(inst);
        }
        return;
    }
    if (expNode->getOperatorType() == ARRAY_INDEX_OP) {
        string offsetUnit = expNode->right->getSymbol();
        string offsetName;
        int unitSize = expNode->left->getType()->getElement()->getSize();

        auto inst = new IRInst(IR_MUL, "tt", offsetUnit, "#" + to_string(unitSize));

        int flag;
        // flag be -1 means only optimized half (one argument), not calculate the real value
        if (Optimizer::cacExpression(inst, &flag)) {
            switch (flag) {
                case 1:
                    offsetName = tempVariable->generateName(tempVariable->allocate());
                    expNode->setSymbol(offsetName);
                    inst->target = offsetName;
                    currIrStatement->addInstruction(inst);
                    break;
                case 2:
                    offsetName = inst->arg1;
                    delete (inst);
                    break;
                default:
                    break;
            }
        } else {
            offsetName = tempVariable->generateName(tempVariable->allocate());
            expNode->setSymbol(offsetName);
            inst->target = offsetName;
            currIrStatement->addInstruction(inst);
        }

        string tempName = tempVariable->generateName(tempVariable->allocate());
        // this instruction can not be optimized, because base addr(left.getSymbol()) is unknown in compile time
        int offset;
        if (isNumber(offsetName, &offset) && offset == 0) {
            currIrStatement->addInstruction(IR_ASSIGN, tempName, expNode->left->getSymbol(), "");
        } else {
            currIrStatement->addInstruction(IR_ADD, tempName, expNode->left->getSymbol(), offsetName);
        }
        if (expNode->isArray() || expNode->getType()->getElementType() == STRUCT_TYPE) {
            expNode->setSymbol(tempName);
        } else {
            expNode->setPointer();
//            currIrStatement->addInstruction(IR_GET_VALUE_IN_ADDRESS, valueName, tempName, "");
            expNode->setSymbol(tempName);
        }

    }
    if (expNode->getOperatorType() == OR_OP) {

    }
    if (expNode->getOperatorType() == AND_OP) {

    }

    auto compareItem = compareOppositeMap.find(expNode->getOperatorType());
    if (compareItem != compareOppositeMap.end()) {
        string leftSymbol, rightSymbol;
        getValueInBinaryExp(expNode->left, expNode->right, &leftSymbol, &rightSymbol);

        string gotolabel = labelStack.back();
        IRInst *jumpInst = currIrStatement->addInstruction(compareItem->second, gotolabel, leftSymbol, rightSymbol);
        addInstToJumpEntry(gotolabel, jumpInst);
        labelStack.push_back(gotolabel);
        return;
    }

}

void IRGenerator::getValueInBinaryExp(Exp *left, Exp *right, string *leftSymbol, string *rightSymbol) {
    if (left->isPointer()) {
        *leftSymbol = tempVariable->generateName(tempVariable->allocate());
        currIrStatement->addInstruction(IR_GET_VALUE_IN_ADDRESS, *leftSymbol, left->getSymbol(), "");
    } else {
        *leftSymbol = left->getSymbol();
    }
    if (right->isPointer()) {
        *rightSymbol = tempVariable->generateName(tempVariable->allocate());
        currIrStatement->addInstruction(IR_GET_VALUE_IN_ADDRESS, *rightSymbol, right->getSymbol(), "");
    } else {
        *rightSymbol = right->getSymbol();
    }
}

void IRGenerator::visit(GetAttributeExp *expNode) {
    expNode->object->accept(this);
    Struct *object = (Struct *) expNode->object->getType();
    int offset = object->getOffset(expNode->getAttrName());
    const string &basePointer = expNode->object->getSymbol();
    string tempName = tempVariable->generateName(tempVariable->allocate());
    if (offset == 0) {
        currIrStatement->addInstruction(IR_ASSIGN, tempName, basePointer, "");
    } else {
        currIrStatement->addInstruction(IR_ADD, tempName, basePointer, "#" + to_string(offset));
    }
    if (expNode->isArray() || expNode->getType()->getElementType() == STRUCT_TYPE) {
        expNode->setSymbol(tempName);
    } else {
        expNode->setPointer();
        expNode->setSymbol(tempName);
    }
}


void IRGenerator::visit(UnaryExp *expNode) {
    expNode->operand->accept(this);
    if (expNode->getOperatorType() == SUB_OP) {
        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        currIrStatement->addInstruction(IR_SUB,
                                        expNode->getSymbol(), "#0", expNode->operand->getSymbol());
    }
}

void IRGenerator::visit(InvokeExp *expNode) {
    for (Exp *exp: expNode->args->getArguments()) {
        exp->accept(this);
    }
    tempVariable->releaseAll();
    Function *function = (Function *) currAst->getScope()->get(expNode->functionName);
    if (function->flag == BUILD_NODE) {
        if (function->getName() == "read") {
            string tempName = tempVariable->generateName(tempVariable->allocate());
            currIrStatement->addInstruction(IR_READ, tempName);
            expNode->setSymbol(tempName);
        } else if (function->getName() == "write") {
            Exp *arg1 = expNode->args->getArguments().front();
            string targetSymbol;
            if (arg1->isPointer()) {
                targetSymbol = tempVariable->generateName(tempVariable->allocate());
                currIrStatement->addInstruction(IR_GET_VALUE_IN_ADDRESS, targetSymbol, arg1->getSymbol(), "");
            } else {
                targetSymbol = arg1->getSymbol();
            }
            currIrStatement->addInstruction(IR_WRITE, targetSymbol);
        }
    } else {
        list<Exp *> &args = expNode->args->getArguments();
        auto item = args.end();
        while (item != args.begin()) {
            item--;
            currIrStatement->addInstruction(IR_ARG, (*item)->getSymbol());
        }
        string tempName = tempVariable->generateName(tempVariable->allocate());
        currIrStatement->addInstruction(IR_CALL, tempName, function->getName(), "");
        expNode->setSymbol(tempName);
    }

}

void IRGenerator::visit(IfStatement *statementNode) {
    string endIfLabel = label->generateName(label->allocate());
    labelStack.push_back(endIfLabel);
    statementNode->getExpression()->accept(this);

    statementNode->ifBody->accept(this);
    if (statementNode->elseBody) {
        string elseLabel = label->generateName(label->allocate());
        IRInst *jumpInst = currIrStatement->addInstruction(IR_GOTO, elseLabel);
        addInstToJumpEntry(elseLabel, jumpInst);

        currIrStatement->addInstruction(IR_LABEL, endIfLabel);
        statementNode->elseBody->accept(this);
        currIrStatement->addInstruction(IR_LABEL, elseLabel);
    } else {
        currIrStatement->addInstruction(IR_LABEL, endIfLabel);
    }
    labelStack.pop_back();
}

void IRGenerator::visit(WhileStatement *statementNode) {
    string startLabel = label->generateName(label->allocate());
    string endLabel = label->generateName(label->allocate());
    labelStack.push_back(endLabel);
    breakStack.push_back(endLabel);
    currIrStatement->addInstruction(IR_LABEL, startLabel);
    continueStack.push_back(startLabel);

    statementNode->getExpression()->accept(this);
    statementNode->loop->accept(this);
    IRInst *jumpInst = currIrStatement->addInstruction(IR_GOTO, startLabel);
    addInstToJumpEntry(startLabel, jumpInst);

    currIrStatement->addInstruction(IR_LABEL, endLabel);

    labelStack.pop_back();
    continueStack.pop_back();
    breakStack.pop_back();
}

void IRGenerator::visit(ReturnStatement *statementNode) {
    currIrStatement->addInstruction(IR_RETURN, statementNode->getExpression()->getSymbol());
    tempVariable->releaseAll();
}

string IRGenerator::getAddress(string &id) {
    auto item = this->symbolAddrTable.find(id);
    if (item != symbolAddrTable.end()) {
        return item->second;
    }
    return id;
}

void IRGenerator::initCurrScopeSymbol(LocalScope *localScope) {
    symbolAddrTable.clear();
//    varVariable->releaseAll(true);
    for (auto item: localScope->getVariables()) {
        string newVarName = varVariable->generateName(varVariable->allocate());
        symbolAddrTable.insert(pair<string, string>(item.first, newVarName));
    }
}

void IRGenerator::checkJumpLinks(IR *ir) {
    list<IRInst *> &list_ir_inst = *ir->getInstructions();
    auto curr = list_ir_inst.begin();
    auto preInst = *curr;
    curr++;
    while (curr != list_ir_inst.end()) {
        IRInst *currInst = *curr;
        if (currInst->irOperator == IR_LABEL && preInst->irOperator == IR_LABEL) {
            JumpEntry *entry = this->jumpMap.find(currInst->target)->second;
            for (IRInst *inst:entry->getJumpInst()) {
                inst->target = preInst->target;
            }
            curr = list_ir_inst.erase(curr);
            continue;
        }
        preInst = currInst;
        curr++;
    }
}

void IRGenerator::addInstToJumpEntry(string &labelName, IRInst *inst) {
    auto item = this->jumpMap.find(labelName);
    if (item != jumpMap.end()) {
        JumpEntry *entry = item->second;
        entry->addInst(inst);
    } else {
        auto *jumpEntry = new JumpEntry(labelName);
        jumpEntry->addInst(inst);
        jumpMap.insert(pair<string, JumpEntry *>(labelName, jumpEntry));
    }
}

void IRGenerator::handleBreakAndContinue(Operator bc, Location *location) {
    if (bc == BREAK_OP) {
        if (breakStack.empty()) {
            this->errorHandler.recordError(location, OTHER_ERROR, "can not break here");
            return;
        }
        string nearestBreak = this->breakStack.back();
        currIrStatement->addInstruction(IR_GOTO, nearestBreak);
    } else if (bc == CONT_OP) {
        if (continueStack.empty()) {
            this->errorHandler.recordError(location, OTHER_ERROR, "can not continue here");
            return;
        }
        string nearestContinue = this->continueStack.back();
        currIrStatement->addInstruction(IR_GOTO, nearestContinue);
    } else
        this->errorHandler.recordError(location, OTHER_ERROR, "not break or continue statement");
}

