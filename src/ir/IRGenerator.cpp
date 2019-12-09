//
// Created by hya on 12/8/19.
//

#include "IRGenerator.h"

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

void TempNameGenerator::releaseAll() {
    for (int i = 0; i < curr_max_id_num; i++) {
        allocated[i] = 0;
    }
}


IRGenerator::IRGenerator() {
    this->label = new TempNameGenerator("LABEL", -1);
    this->tempVariable = new TempNameGenerator("t", 10);
    operatorMap.insert(pair<Operator, IROperator>(ADD_OP, IR_ADD));
    operatorMap.insert(pair<Operator, IROperator>(SUB_OP, IR_SUB));
    operatorMap.insert(pair<Operator, IROperator>(MUL_OP, IR_MUL));
    operatorMap.insert(pair<Operator, IROperator>(DIV_OP, IR_DIV));
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
        function->setIr(complileFunctionBody(function));
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
        currIrStatement->addInstruction(IR_PARAM, para->getName());
    }
    free_all(this->scopeStack);
    this->jumpMap.clear();
    transformStmt(f->getBody());
    checkJumpLinks(jumpMap);
    return currIrStatement;
}

void IRGenerator::checkJumpLinks(map<string, JumpEntry> &maps) {

}

void IRGenerator::visit(BinaryExp *expNode) {
    expNode->left->accept(this);
    expNode->right->accept(this);
    if (expNode->getOperatorType() == ASSIGN_OP) {
        currIrStatement->addInstruction(IR_ASSIGN, expNode->left->getSymbol(), expNode->right->getSymbol(), "");
//        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        tempVariable->releaseAll();
    }
    auto item = operatorMap.find(expNode->getOperatorType());
    if (item != operatorMap.end()) {
        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        currIrStatement->addInstruction(item->second,
                                        expNode->getSymbol(), expNode->left->getSymbol(), expNode->right->getSymbol());
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


void IRGenerator::visit(Variable *variable) {
    if (variable->hasInitializer()) {
        Exp *value = variable->getInitializer();
        value->accept(this);
        currIrStatement->addInstruction(IR_ASSIGN, variable->getName(), value->getSymbol(), "");
//        expNode->setSymbol(tempVariable->generateName(tempVariable->allocate()));
        tempVariable->releaseAll();
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
    statementNode->ifBody->accept(this);
    if(statementNode->elseBody)
        statementNode->elseBody->accept(this);
}

void IRGenerator::visit(WhileStatement *statementNode) {

}

void IRGenerator::visit(ReturnStatement *statementNode) {
    currIrStatement->addInstruction(IR_RETURN, statementNode->getExpression()->getSymbol());
    tempVariable->releaseAll();
}
