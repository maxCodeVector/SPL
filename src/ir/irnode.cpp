//
// Created by hya on 12/8/19.
//

#include "irnode.h"

void IR::write(ostream &os) {
    list<IRInst *> instructions;
    for (IR *block: blocks) {
        for (IRInst *inst: *block->getInstructions()) {
            instructions.push_back(inst);
        }
    }
    for (IRInst *inst: instructions)
        os << inst << endl;
    cerr << "inst number: " << instructions.size() << endl;
}

void IRStatement::addInstruction(IROperator irOperator, const string &target, const string &arg1, const string &arg2) {
    this->instructions.push_back(new IRInst(irOperator, target, arg1, arg2));
}

void IRStatement::addInstruction(IROperator irOperator, const string &target) {
    this->instructions.push_back(new IRInst(irOperator, target));
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
    this->arg1 = target;
}

string IRInst::toString() {
    switch (irOperator) {
        case IR_LABEL:
            return "LABEL " + arg1 + " :";
        case IR_FUNCTION:
            return "FUNCTION " + arg1 + " :";
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
        case IR_ASSIGN_VALUE_IN_ADDRESS:
            break;
        case IR_COPY_VALUE_TO_ADDRESS:
            break;
        case IR_GOTO:
            return "GOTO " + arg1;
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
            return "RETURN " + arg1;
        case IR_DEC:
            return "DEC " + target + " " + arg1;
        case IR_PARAM:
            return "PARAM " + arg1;
        case IR_ARG:
            return "ARG " + arg1;
        case IR_CALL:
            return target + " := CALL " + arg1;
        case IR_READ:
            return "READ " + arg1;
        case IR_WRITE:
            return "WRITE " + arg1;
    }
    return "fuckyou";
}

ostream &operator<<(ostream &os, IRInst &inst) {
    return os << inst.toString();
}

ostream &operator<<(ostream &os, IRInst *inst) {
    return operator<<(os, *inst);
}
