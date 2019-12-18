//
// Created by hya on 12/17/19.
//

#include "mips.h"

void Mips::write(std::ostream &os) {
    os << ".data" << endl;
    os << string(NEWLINE) + R"(: .asciiz "\n"  )" << endl;
    os << ".globl main\n.text" << endl;
    for (MIPS_Instruction *instruction: this->mips) {
        if (instruction->op == MIPS_LABEL)
            os << instruction->to_string() << endl;
        else
            os << "\t" << instruction->to_string() << endl;
    }
}

MIPS_Instruction *Mips::addInstruction(MIPS_Instruction *instruction) {
    this->mips.push_back(instruction);
    return instruction;
}

string MIPS_Instruction::to_string() {
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
        case MIPS_ADD:
            return "add " + dest + ", " + src1 + ", " + src2;
        case MIPS_SUB:
            return "sub " + dest + ", " + src1 + ", " + src2;
        case MIPS_MUL:
            return "mul " + dest + ", " + src1 + ", " + src2;
        case MIPS_DIV:
            return "div " + dest + ", " + src1 + ", " + src2;
        case MIPS_MFLO:
            return "mflo " + dest;
        case MIPS_LW:
            break;
        case MIPS_SW:
            break;
        case MIPS_J:
            break;
        case MIPS_JAL:
            break;
        case MIPS_JR:
            return "jr " + dest;
        case MIPS_BLT:
            break;
        case MIPS_BLE:
            break;
        case MIPS_BGT:
            break;
        case MIPS_BGE:
            break;
        case MIPS_BNE:
            break;
        case MIPS_BEQ:
            break;
        case MIPS_SYSCALL:
            return "syscall";
    }
    return "I known nothing";
}

MIPS_Instruction::MIPS_Instruction(
        MipOperator op,
        const string &dest,
        const string &src1,
        const string &src2) {
    this->op = op;
    this->dest = dest;
    this->src1 = src1;
    this->src2 = src2;
}

MIPS_Instruction::MIPS_Instruction(
        MipOperator op,
        const string &dest,
        const string &src1) {
    this->op = op;
    this->dest = dest;
    this->src1 = src1;
}

MIPS_Instruction::MIPS_Instruction(
        MipOperator op,
        const string &dest) {
    this->op = op;
    this->dest = dest;
}

MIPS_Instruction::MIPS_Instruction(
        MipOperator op) {
    this->op = op;
}