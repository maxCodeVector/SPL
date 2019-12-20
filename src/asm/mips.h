//
// Created by hya on 12/17/19.
//

#ifndef SPL_MIPS_H
#define SPL_MIPS_H

#include <ostream>
#include <string>
#include <list>

#define NEWLINE "_ret"
#define INPUT_HINT "_prompt"


using namespace std;

enum MipsOperator {
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
    MipsOperator op;

    string to_string();

    explicit MIPS_Instruction(MipsOperator op);

    MIPS_Instruction(MipsOperator op, const string &dest);

    MIPS_Instruction(MipsOperator op, const string &dest, const string &src1);

    MIPS_Instruction(MipsOperator op, const string &dest, const string &src1, const string &src2);

};


class Mips {

    list<MIPS_Instruction *> mips;

public:
    MIPS_Instruction *addInstruction(MIPS_Instruction *instruction);

    void write(std::ostream &os);

    void push(int bytes);
};


class AddressDescriptor;

struct Reg {
    string prefix;
    int id;
    bool dirty;
    /**
     * may be null, the corresponding address in memory.
     * some may be null because they do not need to stored in memory
     * (example: temporary variable)
     */
    AddressDescriptor *addr = nullptr;

    string getName();

    void setDirty();

    void removeDirty();

    bool isDirty();
};

#endif //SPL_MIPS_H
