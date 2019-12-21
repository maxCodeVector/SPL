//
// Created by hya on 12/20/19.
//

#include "memory.h"

AddressDescriptor::AddressDescriptor(const string &name, Reg *reg, int offset) {
    this->name = name;
    this->reg = reg;
    this->offset = offset;
    if (reg) {
        reg->addr = this;
    }
}

void AddressDescriptor::setNextUsed(int line) {
    this->next_used.push_front(line);
}

bool AddressDescriptor::isAlive() {
//    if (this->name[0] != 't')
//        return true;
    return !this->next_used.empty();
}

int AddressDescriptor::getNextUsed() {
    return next_used.front();
}

void AddressDescriptor::use() {
    if (!next_used.empty())
        this->next_used.pop_front();
}

void AddressDescriptor::loadToReg(Mips *pMips) {
    if (offset < 0)
        return;
    MIPS_Instruction *lw;
    if (this->forward) {
        // used for $a0, $a1..
        lw = new MIPS_Instruction(MIPS_LW, reg->getName(), "$fp", to_string(offset));
    } else {
        lw = new MIPS_Instruction(MIPS_LW, reg->getName(), "$fp", to_string(-offset));
    }
    pMips->addInstruction(lw);
}

void AddressDescriptor::saveToMemory(Mips *pMips) {
    if (offset < 0)
        return;
    MIPS_Instruction *lw;
    if (this->forward) {
        // used for $a0, $a1..
        lw = new MIPS_Instruction(MIPS_SW, reg->getName(), "$fp", to_string(offset));
    } else {
        lw = new MIPS_Instruction(MIPS_SW, reg->getName(), "$fp", to_string(-offset));
    }
    pMips->addInstruction(lw);
}

bool AddressDescriptor::isUseless() {
    return this->name[0] == 't' && this->next_used.empty();
}

Reg *RegisterAllocator::localAllocate(Mips *mips) {

    for (Reg &reg: temp_regs) {
        if (!reg.addr) {
            return &reg;
        }
    }
    Reg *spill = getRegByLru();
    AddressDescriptor *addr = spill->addr;

    if (spill->isDirty()) {
        if (addr->offset < 0) {
            addr->offset = getSpace();
            mips->push(4);
        }
        addr->saveToMemory(mips);
        spill->removeDirty();
    }
    addr->reg = nullptr;
    spill->addr = nullptr;
    return spill;
}

Reg *RegisterAllocator::getRegByLru() {
    int farthest_used = 0;
    Reg *lru = nullptr;
    for (auto &reg: temp_regs) {
        AddressDescriptor *addr = reg.addr;
        if (addr) {
            if (!addr->isAlive()) {
                return &reg;
            }
            if (farthest_used < addr->getNextUsed()) {
                farthest_used = addr->getNextUsed();
                lru = &reg;
            }
        } else
            return &reg;
    }
    return lru;
}

RegisterAllocator::RegisterAllocator() {
    for (int i = 0; i < reg_number; i++) {
        temp_regs[i].prefix = "$t";
        temp_regs[i].id = i;

        static_regs[i].prefix = "$s";
        static_regs[i].id = i;
    }
    for (int i = 0; i < 4; i++) {
        arg_regs[i].prefix = "$a";
        arg_regs[i].id = i;
    }
    reset();
}

int RegisterAllocator::getSpace() {
    fp_offset++;
    return fp_offset * 4;
}

Reg *RegisterAllocator::allocateArgReg(Mips *mips) {
    for (Reg &reg: arg_regs) {
        if (!reg.addr) {
            return &reg;
        }
    }
    return this->localAllocate(mips);
}

void RegisterAllocator::reset() {
    for (int i = 0; i < reg_number; i++) {
        temp_regs[i].reset();
        static_regs[i].reset();
    }
    for (auto &arg_reg : arg_regs) {
        arg_reg.reset();
    }
    fp_offset = 0;
}

