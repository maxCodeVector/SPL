//
// Created by hya on 12/17/19.
//

#include "mips.h"

void Mips::write(std::ostream &os) {
    os << ".data" << endl;
    os << NEWLINE << R"(: .asciiz "\n"  )" << endl;
    os << INPUT_HINT << R"(: .asciiz "Enter an integer:")" << endl;
    os << ".globl main\n.text" << endl;
    for (MIPS_Instruction *instruction: this->mips) {
        if (instruction->op == MIPS_LABEL)
            os << instruction->toString() << endl;
        else
            os << "\t" << instruction->toString() << endl;
    }
}

MIPS_Instruction *Mips::addInstruction(MIPS_Instruction *instruction) {
    this->mips.push_back(instruction);
    return instruction;
}

void Mips::push(int bytes) {
    this->mips.push_back(new MIPS_Instruction(MIPS_ADDI, "$sp", "$sp", to_string(-bytes)));
}

void Mips::pop(int bytes) {
    this->mips.push_back(new MIPS_Instruction(MIPS_ADDI, "$sp", "$sp", to_string(bytes)));
}

list<MIPS_Instruction *> &Mips::getMips() {
    return this->mips;
}

string MIPS_Instruction::toString() {
    switch (this->op) {

        case MIPS_LABEL:
            return this->dest + ":";
        case MIPS_LI:
            return "li " + dest + ", " + src1;
        case MIPS_LA:
            return "la " + dest + ", " + src1;
        case MIPS_MOVE:
            return "move " + dest + ", " + src1;
        case MIPS_ADDI:
            return "addi " + dest + ", " + src1 + ", " + src2;
        case MIPS_ADD:
            return "add " + dest + ", " + src1 + ", " + src2;
        case MIPS_SUB:
            return "sub " + dest + ", " + src1 + ", " + src2;
        case MIPS_MUL:
            return "mul " + dest + ", " + src1 + ", " + src2;
        case MIPS_DIV:
            return "div " + src1 + ", " + src2;
        case MIPS_MFLO:
            return "mflo " + dest;
        case MIPS_LW:
            return "lw " + dest + ", " + src2 + '(' + src1 + ')';
        case MIPS_SW:
            return "sw " + dest + ", " + src2 + '(' + src1 + ')';
        case MIPS_J:
            return "j " + dest;
        case MIPS_JAL:
            return "jal " + dest;
        case MIPS_JR:
            return "jr " + dest;
        case MIPS_BLT:
            return "blt " + src1 + ", " + src2 + ", " + dest;
        case MIPS_BLE:
            return "ble " + src1 + ", " + src2 + ", " + dest;
        case MIPS_BGT:
            return "bgt " + src1 + ", " + src2 + ", " + dest;
        case MIPS_BGE:
            return "bge " + src1 + ", " + src2 + ", " + dest;
        case MIPS_BNE:
            return "bne " + src1 + ", " + src2 + ", " + dest;
        case MIPS_BEQ:
            return "beq " + src1 + ", " + src2 + ", " + dest;
        case MIPS_SYSCALL:
            return "syscall";
    }
    return "I known nothing";
}

MIPS_Instruction::MIPS_Instruction(
        MipsOperator op,
        const string &dest,
        const string &src1,
        const string &src2) {
    this->op = op;
    this->dest = dest;
    this->src1 = src1;
    this->src2 = src2;
}

MIPS_Instruction::MIPS_Instruction(
        MipsOperator op,
        const string &dest,
        const string &src1) {
    this->op = op;
    this->dest = dest;
    this->src1 = src1;
}

MIPS_Instruction::MIPS_Instruction(
        MipsOperator op,
        const string &dest) {
    this->op = op;
    this->dest = dest;
}

MIPS_Instruction::MIPS_Instruction(
        MipsOperator op) {
    this->op = op;
}

string Reg::getName() {
    return prefix + to_string(id);
}

void Reg::setDirty() {
    this->dirty = true;
}

void Reg::removeDirty() {
    this->dirty = false;
}

bool Reg::isDirty() {
    return dirty;
}

void Reg::reset() {
    dirty = false;
    addr = nullptr;
}
