//
// Created by hya on 12/17/19.
//

#ifndef SPL_CODE_GENERATOR_H
#define SPL_CODE_GENERATOR_H

#include "../ir/irnode.h"
#include "mips.h"


struct Reg {
    string prefix;
    int id;
    bool idle;
    bool dirty;

    string getName();
};

struct Block {
    list<IRInst *>::iterator start;
    list<IRInst *>::iterator end;

    // debug used
    list<IRInst *> getAllIRcode() {
        list<IRInst *> list;
        auto itor = start;
        while (itor != end) {
            list.push_back(*itor);
            itor++;
        }
        return list;
    }
};


class RegisterAllocator {
    static const int reg_number = 10;
    Reg temp_regs[reg_number];


public:
    RegisterAllocator();

    Reg *allocate();
};


class CodeGenerator {
//    list<IRInst *> ir_code;
    RegisterAllocator allocator;
    map<string, Reg *> symbolTable;
    list<Block *> irBlocks;

    void generateProcedure();

    void generateCode(Mips *mips, IRInst *inst);

    Reg *getRegOfSymbol(const string &varName);

    void findBlocks(list<IRInst *> &irList);

    void generateBlockCode(Mips *pMips, Block *pBlock);

    void generateAddSub(Mips *pMips, IRInst *pInst);

    void generateAssign(Mips *mips, const IRInst *inst);

    void generateMultiply(Mips *pMips, IRInst *pInst);

    void generateWrite(Mips *pMips, IRInst *pInst);

    void generateReturn(Mips *pMips, IRInst *pInst);

    void generateRead(Mips *pMips, IRInst *pInst);


public:
    explicit CodeGenerator(IR *ir);

    Mips *generateMipsCode();

};


#endif //SPL_CODE_GENERATOR_H
