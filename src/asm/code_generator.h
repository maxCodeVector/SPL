//
// Created by hya on 12/17/19.
//

#ifndef SPL_CODE_GENERATOR_H
#define SPL_CODE_GENERATOR_H

#include "../ir/irnode.h"
#include "mips.h"

struct AddressDescriptor {
    /** the reg that store this address's value now
     * may be null because they are in memory not in register currently
     */
    Reg *reg;
    /** the offset relative by current stack pointer
     * if it only appeared in register, then offset is negative
     */
    int offset;
private:
    string name;
    list<int> next_used;
public:
    bool isAlive();

    void setNextUsed(int line);

    void use();

    int getNextUsed();

    AddressDescriptor(const string &name, Reg *reg, int offset);
};

struct Block {
    list<IRInst *>::iterator start;
    list<IRInst *>::iterator end;
    int numOfInst = 0;

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

    void analysisBlock(map<string, AddressDescriptor *> &symbolTable);

};


class RegisterAllocator {
    static const int reg_number = 3;
    Reg temp_regs[reg_number];
    int fp_offset;

    int getSpace();

    Reg *getRegByLru();

public:
    RegisterAllocator();

    Reg *localAllocate(Mips *mips);

};


class CodeGenerator {
//    list<IRInst *> ir_code;
    Mips *mips;
    RegisterAllocator allocator;
    map<string, AddressDescriptor *> symbolTable;
    list<Block *> irBlocks;

    void generateCode(Mips *mips, IRInst *inst);

    Reg *getRegOfSymbol(const string &varName);

    void findBlocks(list<IRInst *> &irList);

    void generateBlockCode(Mips *pMips, Block *pBlock);

    void generateArithmetic(Mips *pMips, IRInst *pInst);

    void generateAssign(Mips *mips, const IRInst *inst);

    void generateWrite(Mips *pMips, IRInst *pInst);

    void generateReturn(Mips *pMips, IRInst *pInst);

    void generateRead(Mips *pMips, IRInst *pInst);


public:
    explicit CodeGenerator(IR *ir);

    Mips *generateMipsCode();

    void generateBranch(Mips *pMips, IRInst *pInst);
};


#endif //SPL_CODE_GENERATOR_H
