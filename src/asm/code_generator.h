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
    /** the offset relative by current stack pointer ($fp), noticed that it is different
     * from $sp address
     * if it only appeared in register, then offset is negative
     */
    int offset;
    /**
     * default is false, true means the address is in the opposite of increased direction of $fp
     */
    bool forward = false;
private:
    string name;
    list<int> next_used;
public:
    bool isAlive();

    void setNextUsed(int line);

    void use();

    int getNextUsed();

    AddressDescriptor(const string &name, Reg *reg, int offset);

    void loadToReg(Reg *pReg, Mips *pMips);
};

class RegisterAllocator {
    static const int reg_number = 10;
    Reg temp_regs[reg_number];
    Reg static_regs[reg_number];
    Reg arg_regs[4];
    int fp_offset;

    int getSpace();

    Reg *getRegByLru();

public:
    RegisterAllocator();

    Reg *localAllocate(Mips *mips);

    Reg *allocateArgReg(Mips *mips);

};


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

    void savaRegisterStatus(Mips *pMips) const;

    Reg *getRegOfSymbol(const string &varName);

    void bindingArgumentRegister(Reg *arg, string& argName, int offset) const;


public:

    list<IRInst *>::iterator start;
    list<IRInst *>::iterator end;
    int numOfInst = 0;

    explicit Block(RegisterAllocator *allocator, map<string, AddressDescriptor *> &symbolTable);

    void setMips(Mips *mips);

    void analysis();

    void generateCode();

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
