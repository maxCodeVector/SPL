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

void TempNameGenerator::releaseAll(bool strong= false) {
    if (allocated) {
        for (int i = 0; i < curr_max_id_num; i++) {
            allocated[i] = 0;
        }
    } else if(strong){
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
    exprNestLevel++;
    exp->accept(this);
    exprNestLevel--;
    return nullptr;
}

bool IRGenerator::isStatement() {
    return exprNestLevel == 0;
}

IR *IRGenerator::generate(AST &ast) {
    currAst = &ast;
    for (Variable *variable: ast.getDefinedVars()) {
        if (variable->hasInitializer()) {
            variable->setIR(transformExpr(variable->getInitializer()));
        }
    }
    for (Function *function:ast.getFunctions()) {
        initCurrScopeSymbol ((LocalScope*)function->getScope());
        Optimizer irOptimizer;
        IRStatement *insts = complileFunctionBody(function);
        function->setIr(irOptimizer.optimize(insts));
    }
    if (this->errorHandler.errorOccured()) {
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
    checkJumpLinks(jumpMap);
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
        currIrStatement->addInstruction(IR_ADDRESS,getAddress(variable->getName()), object, "");
        return;
    }
}

void IRGenerator::checkJumpLinks(map<string, JumpEntry *> &maps) {

}

void IRGenerator::visit(BinaryExp *expNode) {
    expNode->left->accept(this);
    expNode->right->accept(this);
    if (expNode->getOperatorType() == ASSIGN_OP) {
        currIrStatement->addInstruction(IR_ASSIGN, expNode->left->getSymbol(), expNode->right->getSymbol(), "");
//        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        tempVariable->releaseAll();
        return;
    }
    auto item = arithmeticMap.find(expNode->getOperatorType());
    if (item != arithmeticMap.end()) {
        auto inst = IRInst(item->second, "tt", expNode->left->getSymbol(), expNode->right->getSymbol());
        int value;
        if (Optimizer::cacExpression(&inst, &value)) {
            expNode->setSymbol("#" + to_string(value));
        } else {
            expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
            currIrStatement->addInstruction(item->second, expNode->getSymbol(),
                                            expNode->left->getSymbol(), expNode->right->getSymbol());
        }
        return;
    }
    if (expNode->getOperatorType() == ARRAY_INDEX_OP) {
        string offsetUnit = expNode->right->getSymbol();
        string offsetName = tempVariable->generateName(tempVariable->allocate());
        int unitSize = expNode->left->getType()->getElement()->getSize();
        currIrStatement->addInstruction(IR_MUL, offsetName, offsetUnit, "#" + to_string(unitSize));

        string tempName = tempVariable->generateName(tempVariable->allocate());
        currIrStatement->addInstruction(IR_ADD, tempName, expNode->left->getSymbol(), offsetName);
        if (expNode->isArray() || expNode->getType()->getElementType() == STRUCT_TYPE) {
            expNode->setSymbol(tempName);
        } else
            expNode->setSymbol("*" + tempName);

    }
    if (expNode->getOperatorType() == OR_OP) {

    }
    if (expNode->getOperatorType() == AND_OP) {

    }

    auto compareItem = compareOppositeMap.find(expNode->getOperatorType());
    if (compareItem != compareOppositeMap.end()) {
        string gotolabel = label->generateName(label->allocate());
        auto *jumpEntry = new JumpEntry;
        this->jumpMap.insert(pair<string, JumpEntry *>(gotolabel, jumpEntry));
        currIrStatement->addInstruction(compareItem->second,
                                        gotolabel, expNode->left->getSymbol(), expNode->right->getSymbol());
        labelStack.push_back(gotolabel);
        return;
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
            currIrStatement->addInstruction(IR_WRITE, expNode->args->getArguments().front()->getSymbol());
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
    statementNode->getExpression()->accept(this);
    string gotoLabel = labelStack.back();
    labelStack.pop_back();
    statementNode->ifBody->accept(this);
    if (statementNode->elseBody) {
        string outLabel = label->generateName(label->allocate());
        currIrStatement->addInstruction(IR_GOTO, outLabel);
        currIrStatement->addInstruction(IR_LABEL, gotoLabel);
        statementNode->elseBody->accept(this);
        currIrStatement->addInstruction(IR_LABEL, outLabel);
    } else {
        currIrStatement->addInstruction(IR_LABEL, gotoLabel);
    }
}

void IRGenerator::visit(WhileStatement *statementNode) {
    string startLabel = label->generateName(label->allocate());
    currIrStatement->addInstruction(IR_LABEL, startLabel);
    statementNode->getExpression()->accept(this);
    statementNode->loop->accept(this);
    currIrStatement->addInstruction(IR_GOTO, startLabel);

    string gotoLabel = labelStack.back();
    labelStack.pop_back();
    currIrStatement->addInstruction(IR_LABEL, gotoLabel);
}

void IRGenerator::visit(ReturnStatement *statementNode) {
    currIrStatement->addInstruction(IR_RETURN, statementNode->getExpression()->getSymbol());
    tempVariable->releaseAll();
}

void IRGenerator::visit(GetAttributeExp *expNode) {
    expNode->object->accept(this);
    Struct *object = (Struct *) expNode->object->getType();
    int offset = object->getOffset(expNode->getAttrName());
    const string &basePointer = expNode->object->getSymbol();
    string tempName = tempVariable->generateName(tempVariable->allocate());
    currIrStatement->addInstruction(IR_ADD, tempName, basePointer, "#" + to_string(offset));
    if (expNode->isArray() || expNode->getType()->getElementType() == STRUCT_TYPE) {
        expNode->setSymbol(tempName);
    } else
        expNode->setSymbol("*" + tempName);
}

string IRGenerator::getAddress(string &id) {
    auto item = this->symbolAddrTable.find(id);
    if(item!=symbolAddrTable.end()){
        return item->second;
    }
    return id;
}

void IRGenerator::initCurrScopeSymbol(LocalScope *localScope) {
    symbolAddrTable.clear();
//    varVariable->releaseAll(true);
    for(auto item: localScope->getVariables()){
        string newVarName = varVariable->generateName(varVariable->allocate());
        symbolAddrTable.insert(pair<string, string>(item.first, newVarName));
    }
}
