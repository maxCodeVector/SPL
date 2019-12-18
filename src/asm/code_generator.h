//
// Created by hya on 12/17/19.
//

#ifndef SPL_CODE_GENERATOR_H
#define SPL_CODE_GENERATOR_H

#include "../ir/irnode.h"
#include "mips.h"


struct Reg {
    int id;
    bool dirty;

    string getName();
};


class RegisterAllocator {
    static const int reg_number = 10;
    string prefix;
    Reg temp_regs[reg_number];


public:
    RegisterAllocator();

    Reg *allocate();
};


class CodeGenerator {
    list<IRInst *> ir_code;
    RegisterAllocator allocator;
    map<string, Reg *> symbolTable;

    void generateProcedure();

    void generateCode(Mips *mips, IRInst *inst);

    Reg * getRegOfSymbol(const string &varName);

public:
    explicit CodeGenerator(IR *ir);

    Mips *generateMipsCode();

    void generateAddSub(Mips *pMips, IRInst *pInst);

    void generateAssign(Mips *mips, const IRInst *inst);

    void generateMultiply(Mips *pMips, IRInst *pInst);

    void generateWrite(Mips *pMips, IRInst *pInst);

    void generateReturn(Mips *pMips, IRInst *pInst);
};


#endif //SPL_CODE_GENERATOR_H
