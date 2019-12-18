//
// Created by hya on 12/17/19.
//

#ifndef SPL_MIPS_H
#define SPL_MIPS_H

#include <ostream>
#include <string>
#include <list>
#define NEWLINE "_ret"


using namespace std;

enum MipOperator {
    MIPS_LABEL,
    MIPS_LI,
    MIPS_LA,
    MIPS_MOVE,
    MIPS_ADDI,
    MIPS_ADD,
    MIPS_SUB,
    MIPS_MUL,
    MIPS_DIV,
    MIPS_MFLO,
    MIPS_LW,
    MIPS_SW,
    MIPS_J,
    MIPS_JAL,
    MIPS_JR,
    MIPS_BLT,
    MIPS_BLE,
    MIPS_BGT,
    MIPS_BGE,
    MIPS_BNE,
    MIPS_BEQ,
    MIPS_SYSCALL
};


class MIPS_Instruction {
    string dest;
    string src1;
    string src2;

public:
    MipOperator op;

    string to_string();

    explicit MIPS_Instruction(MipOperator op);

    MIPS_Instruction(MipOperator op, const string &dest);

    MIPS_Instruction(MipOperator op, const string &dest, const string &src1);

    MIPS_Instruction(MipOperator op, const string &dest, const string &src1, const string &src2);

};


class Mips {

    list<MIPS_Instruction *> mips;

public:
    MIPS_Instruction *addInstruction(MIPS_Instruction *instruction);

    void write(std::ostream &os);
};


#endif //SPL_MIPS_H
