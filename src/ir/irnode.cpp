//
// Created by hya on 12/8/19.
//

#include "optimizer.h"
#include "IRGenerator.h"
#include "irnode.h"

void IR::write(ostream &os) {
    int numOfInsts = 0;
    for (IR *block: blocks) {
        for (IRInst *inst: *block->getInstructions()) {
            numOfInsts++;
            os << inst << endl;
        }
    }
    cerr << "inst number: " << numOfInsts << endl;
}


const list<IR *> &IR::getBlocks() const {
    return this->blocks;
}

IRInst *
IRStatement::addInstruction(IROperator irOperator, const string &target, const string &arg1, const string &arg2) {
    IRInst *inst = new IRInst(irOperator, target, arg1, arg2);
    this->instructions.push_back(inst);
    return inst;
}

IRInst *IRStatement::addInstruction(IROperator irOperator, const string &target) {
    IRInst *inst = new IRInst(irOperator, target);
    this->instructions.push_back(inst);
    return inst;
}

list<IRInst *> *IRStatement::getInstructions() {
    return &this->instructions;
}

void IRStatement::addInstruction(IRInst *inst) {
    this->instructions.push_back(inst);
}

void IRExpr::addInstruction(IROperator irOperator, string &target, string &arg1, string &arg2) {
    this->instructions.push_back(new IRInst(irOperator, target, arg1, arg2));
}

void IRExpr::addInstruction(IROperator irOperator, string &target) {
    this->instructions.push_back(new IRInst(irOperator, target));
}

list<IRInst *> *IRExpr::getInstructions() {
    return &this->instructions;
}


void IR::addBlock(IR *ir) {
    blocks.push_back(ir);
}

IRInst::IRInst(IROperator irOperator, const string &target, const string &arg1, const string &arg2) {
    this->irOperator = irOperator;
    this->target = target;
    this->arg1 = arg1;
    this->arg2 = arg2;
}

IRInst::IRInst(IROperator irOperator, const string &target) {
    this->irOperator = irOperator;
    this->target = target;
}

string IRInst::toString() const {
    switch (irOperator) {
        case IR_LABEL:
            return "LABEL " + target + " :";
        case IR_FUNCTION:
            return "FUNCTION " + target + " :";
        case IR_ASSIGN:
            return target + " := " + arg1;
        case IR_ADD:
            return target + " := " + arg1 + " + " + arg2;
        case IR_SUB:
            return target + " := " + arg1 + " - " + arg2;
        case IR_MUL:
            return target + " := " + arg1 + " * " + arg2;
        case IR_DIV:
            return target + " := " + arg1 + " / " + arg2;
        case IR_ADDRESS:
            return target + " := &" + arg1;
        case IR_GET_VALUE_IN_ADDRESS:
            return target + " := *" + arg1;
        case IR_COPY_VALUE_TO_ADDRESS:
            return "*" + target + " := " + arg1;
        case IR_GOTO:
            return "GOTO " + target;
        case IR_IF_LT:
            return "IF " + arg1 + " < " + arg2 + " GOTO " + target;
        case IR_IF_LE:
            return "IF " + arg1 + " <= " + arg2 + " GOTO " + target;
        case IR_IF_GT:
            return "IF " + arg1 + " > " + arg2 + " GOTO " + target;
        case IR_IF_GE:
            return "IF " + arg1 + " >= " + arg2 + " GOTO " + target;
        case IR_IF_EQ:
            return "IF " + arg1 + " == " + arg2 + " GOTO " + target;
        case IR_IF_NE:
            return "IF " + arg1 + " != " + arg2 + " GOTO " + target;
        case IR_RETURN:
            return "RETURN " + target;
        case IR_DEC:
            return "DEC " + target + " " + arg1;
        case IR_PARAM:
            return "PARAM " + target;
        case IR_ARG:
            return "ARG " + target;
        case IR_CALL:
            return target + " := CALL " + arg1;
        case IR_READ:
            return "READ " + target;
        case IR_WRITE:
            return "WRITE " + target;
    }
    return "fuckyou";
}

ostream &operator<<(ostream &os, IRInst &inst) {
    return os << inst.toString();
}

ostream &operator<<(ostream &os, IRInst *inst) {
    return operator<<(os, *inst);
}

JumpEntry::JumpEntry(const string &labelName) {
    this->labelName = labelName;
}

void JumpEntry::addInst(IRInst *inst) {
    this->jumpInst.push_back(inst);
}

const list<IRInst *> &JumpEntry::getJumpInst() const {
    return jumpInst;
}

void JumpEntry::removeInst(IRInst *inst) {
    this->jumpInst.remove(inst);
}

bool isSimilarGoto(IROperator irOperator) {
    return irOperator == IR_GOTO
           || irOperator == IR_IF_LT
           || irOperator == IR_IF_LE
           || irOperator == IR_IF_GT
           || irOperator == IR_IF_GE
           || irOperator == IR_IF_EQ
           || irOperator == IR_IF_NE;
}