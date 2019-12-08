//
// Created by hya on 12/8/19.
//

#include "irnode.h"

void IR::write(ostream &os) {
    list<IRInst*> instructions;
    for(IR* block: blocks){
        for(IRInst* inst: *block->getInstructions()){
            instructions.push_back(inst);
        }
    }
    for(IRInst* inst: instructions)
        os << inst;
}

void IRStatement ::addInstruction(IROperator irOperator, string &target, string &arg1, string &arg2) {
    this->instructions.push_back(new IRInst(irOperator, target, arg1, arg2));
}

void IRStatement::addInstruction(IROperator irOperator, string &target) {
    this->instructions.push_back(new IRInst(irOperator, target));
}

list<IRInst *> *IRStatement::getInstructions() {
    return &this->instructions;
}


void IR::addBlock(IR *ir) {
    blocks.push_back(ir);
}

IRInst::IRInst(IROperator irOperator, string &target, string &arg1, string &arg2) {
    this->irOperator = irOperator;
    this->target = target;
    this->arg1 = arg1;
    this->arg2 = arg2;
}

IRInst::IRInst(IROperator irOperator, string &target) {
    this->irOperator = irOperator;
    this->target = target;
}

string IRInst::toString() {
    if(this->irOperator==IR_FUNCTION)
        return "FUNCTION " + target + " :";
    return "fuckyou";
}

ostream &operator<<(ostream & os, IRInst& inst) {
    return os << inst.toString();
}

ostream &operator<<(ostream & os, IRInst* inst) {
    return operator<<(os, *inst);
}
