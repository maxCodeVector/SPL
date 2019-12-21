//
// Created by hya on 12/17/19.
//

#ifndef SPL_CODE_GENERATOR_H
#define SPL_CODE_GENERATOR_H

#include "../ir/irnode.h"
#include "mips.h"
#include "memory.h"


class Block {

    RegisterAllocator *allocator;
    Mips *mips = nullptr;
    map<string, AddressDescriptor *> &symbolTable;

private:
    list<IRInst *> arguments_of_next_call;
    list<IRInst *> parameters_of_function;

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

    void generateCode(IRInst *inst);

    void generateArithmetic(Mips *pMips, IRInst *pInst);

    void generateAssign(Mips *mips, const IRInst *inst);

    void generateWrite(Mips *pMips, IRInst *pInst);

    void generateReturn(Mips *pMips, IRInst *pInst);

    void generateRead(Mips *pMips, IRInst *pInst);

    void generateBranch(Mips *pMips, IRInst *pInst);

    void generateCallee(Mips *mips, const IRInst *inst) const;

    void generateCaller(Mips *pMips, IRInst *pInst);

    void generateArgument(Mips *pMips, IRInst *pInst);

    void generateParameter(Mips *pMips, IRInst *pInst);

    Reg *getRegOfSymbol(const string &varName);

    void bindingArgumentRegister(Reg *arg, string &argName, int offset) const;

    void restoreRegisterStatus(Mips *pMips);

    void saveRegisterStatus(Mips *pMips) const;

public:

    list<IRInst *>::iterator start;
    list<IRInst *>::iterator end;
    int numOfInst = 0;

    explicit Block(RegisterAllocator *allocator, map<string, AddressDescriptor *> &symbolTable);

    void setMips(Mips *mips);

    void analysis();

    void generateCode();

    void saveRegisterArg0(Mips *pMips);

    void restoreRegisterArg0(Mips *pMips);
};


class CodeGenerator {

//    list<IRInst *> ir_code;
    Mips *mips = nullptr;
    RegisterAllocator allocator;
    list<Block *> irBlocks;
    map<string, AddressDescriptor *> currSymbolTable;

    void findBlocks(list<IRInst *> &irList);

public:
    explicit CodeGenerator(IR *ir);

    Mips *generateMipsCode();

};

#endif //SPL_CODE_GENERATOR_H
