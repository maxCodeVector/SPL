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

string IRInst::toString() {
    if (this->irOperator == IR_FUNCTION)
        return "FUNCTION " + target + " :";
    if (this->irOperator == IR_ASSIGN)
        return target + " := " + arg1;
    if (this->irOperator == IR_RETURN)
        return "RETURN " + target;

    if (this->irOperator == IR_ADD)
        return target + " := " + arg1 + " + " + arg2;
    if (this->irOperator == IR_SUB)
        return target + " := " + arg1 + " - " + arg2;
    if (this->irOperator == IR_MUL)
        return target + " := " + arg1 + " * " + arg2;
    if (this->irOperator == IR_DIV)
        return target + " := " + arg1 + " / " + arg2;

    if (this->irOperator == IR_WRITE)
        return "WRITE " + target;
    if (this->irOperator == IR_READ)
        return "READ " + target;

    if (this->irOperator == IR_CALL)
        return target + " := CALL " + arg1;
    if (this->irOperator == IR_ARG)
        return "ARG " + target;
    if (this->irOperator == IR_PARAM)
        return "PARAM " + target;

    if (this->irOperator == IR_IF_EQ)
        return "IF " + arg1 + " == " + arg2 + " GOTO " + target;
    if (this->irOperator == IR_IF_NE)
        return "IF " + arg1 + " != " + arg2 + " GOTO " + target;
    if (this->irOperator == IR_IF_LT)
        return "IF " + arg1 + " < " + arg2 + " GOTO " + target;
    if (this->irOperator == IR_IF_LE)
        return "IF " + arg1 + " <= " + arg2 + " GOTO " + target;
    if (this->irOperator == IR_IF_GT)
        return "IF " + arg1 + " > " + arg2 + " GOTO " + target;
    if (this->irOperator == IR_IF_GE)
        return "IF " + arg1 + " >= " + arg2 + " GOTO " + target;
    if (this->irOperator == IR_GOTO)
        return "GOTO " + target;
    if (this->irOperator == IR_LABEL)
        return "LABEL " + target + " :";

    if (this->irOperator == IR_DEC)
        return "DEC " + target + " " +arg1;
    if (this->irOperator == IR_ADDRESS)
        return target + " := &" +arg1;

    target + arg1;
    return "fuckyou";
}

ostream &operator<<(ostream &os, IRInst &inst) {
    return os << inst.toString();
}

ostream &operator<<(ostream &os, IRInst *inst) {
    return operator<<(os, *inst);
}
