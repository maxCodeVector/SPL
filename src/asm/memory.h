//
// Created by hya on 12/20/19.
//

#ifndef SPL_MEMORY_H
#define SPL_MEMORY_H

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

    void loadToReg(Mips *pMips);

    void saveToMemory(Mips *pMips);

    bool isUseless();
};

class RegisterAllocator {
    static const int reg_number = 3;
    Reg temp_regs[reg_number];
    Reg static_regs[reg_number];
    Reg arg_regs[4];
    int fp_offset;


    Reg *getRegByLru();

public:
    RegisterAllocator();

    Reg *localAllocate(Mips *mips);

    Reg *allocateArgReg(Mips *mips);

    int getSpace();

    void reset();

};


#endif //SPL_MEMORY_H
