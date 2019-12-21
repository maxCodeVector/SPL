//
// Created by hya on 12/17/19.
//

#include "code_generator.h"
#include "../ir/optimizer.h"


void increaseUse(const string &var, map<string, AddressDescriptor *> &symbolTable) {
    if (var[0] == '#') {
        return;
    }
    auto item = symbolTable.find(var);
    AddressDescriptor *addr = item->second;
    addr->use();
}


CodeGenerator::CodeGenerator(IR *ir) {
    for (auto block: ir->getBlocks()) {
        findBlocks(*block->getInstructions());
    }
}

void CodeGenerator::findBlocks(list<IRInst *> &irList) {
    auto itor = irList.begin();
    Block *block = new Block(&allocator, currSymbolTable);
    block->start = itor;
    while (itor != irList.end()) {
        IRInst *inst = *itor;
        if (inst->irOperator == IR_LABEL) {
            block->end = itor;
            this->irBlocks.push_back(block);
            block = new Block(&allocator, currSymbolTable);
            block->start = itor;
        } else if (itor != irList.begin()) {
            itor--;
            IRInst *pre = *itor;
            itor++;
            if (isSimilarGoto(pre->irOperator)) {
                block->end = itor;
                this->irBlocks.push_back(block);
                block = new Block(&allocator, currSymbolTable);
                block->start = itor;
            }
        }
        block->numOfInst++;
        itor++;
    }
    if (block->start != itor) {
        block->end = itor;
        this->irBlocks.push_back(block);
    } else
        delete (block);
}


Mips *CodeGenerator::generateMipsCode() {
    Mips *mips = new Mips;
    auto iter = this->irBlocks.begin();
    while (iter != this->irBlocks.end()) {
        Block *block = (*iter);
        block->setMips(mips);
        block->generateCode();
        iter++;
    }
    return mips;
}


void Block::generateCode() {
    this->analysis();
    auto &itor = start;
    const auto &temp = getAllIRcode();
    while (itor != end) {
        generateCode(*itor);
        itor++;
    }
    if (!isSaved) {
        saveRegisterStatus(mips);
    }
}

void Block::generateCode(IRInst *inst) {
    if (isSimilarGoto(inst->irOperator)) {
        saveRegisterStatus(mips);
        this->isSaved = true;
    }
    switch (inst->irOperator) {

        case IR_LABEL:
            mips->addInstruction(new MIPS_Instruction{MIPS_LABEL, inst->target});
            break;
        case IR_FUNCTION:
            generateCallee(mips, inst);
            break;
        case IR_ASSIGN:
            generateAssign(mips, inst);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            generateArithmetic(mips, inst);
            break;
        case IR_ADDRESS:
            break;
        case IR_GET_VALUE_IN_ADDRESS:
            break;
        case IR_COPY_VALUE_TO_ADDRESS:
            break;
        case IR_GOTO:
            mips->addInstruction(new MIPS_Instruction{MIPS_J, inst->target});
            break;
        case IR_IF_LT:
        case IR_IF_LE:
        case IR_IF_GT:
        case IR_IF_GE:
        case IR_IF_EQ:
        case IR_IF_NE:
            generateBranch(mips, inst);
            break;
        case IR_RETURN:
            generateReturn(mips, inst);
            break;
        case IR_DEC:
            break;
        case IR_PARAM:
            break;
        case IR_ARG:
            this->arguments_of_next_call.push_back(inst);
            break;
        case IR_CALL:
            generateCaller(mips, inst);
            break;
        case IR_READ:
            generateRead(mips, inst);
            break;
        case IR_WRITE:
            generateWrite(mips, inst);
            break;
    }

}

void Block::generateCaller(Mips *pMips, IRInst *pInst) {
    /**
     * saves the register values and other status info, not including $ra,
     * $ra's value should be stored in caller, and loaded in return
     */
    saveRegisterStatus(mips);
    /**
     * other space used to store control link, access link, in particular
     * 1. current top_sp ($fp)
     * 2. return address ($ra)
     * we do not need to leave space to store return value,
     * because we only return integer, so put it in $v0 directly
     */
    int numOfArgs = this->arguments_of_next_call.size();
    pMips->push(4 * numOfArgs + 8);
    pMips->addInstruction(new MIPS_Instruction(MIPS_SW, "$fp", "$sp", "0"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SW, "$ra", "$sp", "4"));
    int arg_count = 2;
    for (IRInst *inst: arguments_of_next_call) {
        // todo use $a0, $a1, $a2, $a3
        int value;
        Reg *arg;
        if (isNumber(inst->target, &value)) {
            arg = allocator->localAllocate(mips);
            mips->addInstruction(new MIPS_Instruction(MIPS_LI, arg->getName(), to_string(value)));
        } else {
            arg = getRegOfSymbol(inst->target);
            increaseUse(inst->target, this->symbolTable);
        }
        pMips->addInstruction(new MIPS_Instruction(MIPS_SW, arg->getName(), "$sp", to_string(arg_count * 4)));
        arg_count++;
    }
    this->arguments_of_next_call.clear();


    pMips->addInstruction(new MIPS_Instruction(MIPS_JAL, pInst->arg1));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LW, "$ra", "$fp", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LW, "$fp", "$fp", "0"));

    /** restore register value from memory*/
    restoreRegisterStatus(mips);
    Reg *ret = getRegOfSymbol(pInst->target);
    ret->setDirty();
    pMips->addInstruction(new MIPS_Instruction(MIPS_MOVE, ret->getName(), "$v0"));

    pMips->pop(4 * numOfArgs + 8);
}


void Block::generateCallee(Mips *mips, const IRInst *inst) const {
    auto *mipsInstruction = new MIPS_Instruction{MIPS_LABEL, inst->target};
    mips->addInstruction(mipsInstruction);
    /** update top_sp ($fp) */
    mips->addInstruction(new MIPS_Instruction(MIPS_MOVE, "$fp", "$sp"));

    int numberOfPara = parameters_of_function.size();
    int num_arg_offset = 2 + numberOfPara;
    for (IRInst *para: parameters_of_function) {
        num_arg_offset--;
        Reg *arg = allocator->allocateArgReg(mips);
        // sometimes arg has in binding the address
        if (!arg->addr) {
            bindingArgumentRegister(arg, para->target, num_arg_offset * 4);
        }
    }
}

void Block::generateReturn(Mips *pMips, IRInst *pInst) {
    int value;
    if (isNumber(pInst->target, &value)) {
        auto move = new MIPS_Instruction(MIPS_LI, "$v0", to_string(value));
        pMips->addInstruction(move);
    } else {
        auto src = getRegOfSymbol(pInst->target);
        increaseUse(pInst->target, symbolTable);
        auto move = new MIPS_Instruction(MIPS_MOVE, "$v0", src->getName());
        pMips->addInstruction(move);
    }
    mips->addInstruction(new MIPS_Instruction(MIPS_MOVE, "$sp", "$fp"));
    auto jr = new MIPS_Instruction(MIPS_JR, "$ra");
    pMips->addInstruction(jr);
}


void Block::generateAssign(Mips *mips, const IRInst *inst) {
    Reg *dest = getRegOfSymbol(inst->target);
    int value;
    if (isNumber(inst->arg1, &value)) {
        auto *mipsInst = new MIPS_Instruction(MIPS_LI, dest->getName(), to_string(value));
        mipsInst->setComments(inst->toString());
        mips->addInstruction(mipsInst);
    } else {
        Reg *src = getRegOfSymbol(inst->arg1);
        increaseUse(inst->arg1, symbolTable);
        auto *mipsInst = new MIPS_Instruction(MIPS_MOVE, dest->getName(), src->getName());
        mips->addInstruction(mipsInst);
        mipsInst->setComments(inst->toString());
        increaseUse(inst->arg1, symbolTable);
    }
    dest->setDirty();
}


void Block::generateArithmetic(Mips *pMips, IRInst *pInst) {
    int value;
    Reg *src1, *src2;
    if (isNumber(pInst->arg1, &value)) {
        src1 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src1->getName(), to_string(value));
        pMips->addInstruction(inst);
        src1->addr = allocator->CONSTANT;
    } else
        src1 = getRegOfSymbol(pInst->arg1);

    if (isNumber(pInst->arg2, &value)) {
        src2 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src2->getName(), to_string(value));
        pMips->addInstruction(inst);
        src2->addr = allocator->CONSTANT;
    } else
        src2 = getRegOfSymbol(pInst->arg2);

    Reg *dest = getRegOfSymbol(pInst->target);
    switch (pInst->irOperator) {
        case IR_ADD: {
            auto add = new MIPS_Instruction(MIPS_ADD, dest->getName(), src1->getName(), src2->getName());

            add->setComments(pInst->toString());
            pMips->addInstruction(add);
        }
            break;
        case IR_SUB: {
            auto sub = new MIPS_Instruction(MIPS_SUB, dest->getName(), src1->getName(), src2->getName());

            sub->setComments(pInst->toString());
            pMips->addInstruction(sub);
        }
            break;
        case IR_MUL: {
            auto mul = new MIPS_Instruction(MIPS_MUL, dest->getName(), src1->getName(), src2->getName());

            mul->setComments(pInst->toString());
            pMips->addInstruction(mul);
        }
            break;
        case IR_DIV: {
            auto mul = new MIPS_Instruction(MIPS_DIV, "", src1->getName(), src2->getName());
            auto mflo = new MIPS_Instruction(MIPS_MFLO, dest->getName());

            mflo->setComments(pInst->toString());
            pMips->addInstruction(mul);
            pMips->addInstruction(mflo);
        }
            break;
        default:
            break;
    }
    increaseUse(pInst->arg1, symbolTable);
    increaseUse(pInst->arg2, symbolTable);
    if (src1->addr == allocator->CONSTANT) {
        src1->addr = nullptr;
    }
    if (src2->addr == allocator->CONSTANT) {
        src2->addr = nullptr;
    }
    dest->setDirty();
}

void Block::generateRead(Mips *pMips, IRInst *pInst) {
    saveRegisterArg0(mips);

    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LA, "$a0", INPUT_HINT));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));

    Reg *num = getRegOfSymbol(pInst->target);
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "5"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));

    auto *getRetV = new MIPS_Instruction(MIPS_MOVE, num->getName(), "$v0");
    getRetV->setComments(pInst->toString());
    pMips->addInstruction(getRetV);
    num->setDirty();

}

void Block::generateWrite(Mips *pMips, IRInst *pInst) {
    saveRegisterArg0(mips);

    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "1"));
    int value;
    if (isNumber(pInst->target, &value)) {
        pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$a0", to_string(value)));
    } else {
        Reg *num = getRegOfSymbol(pInst->target);
        increaseUse(pInst->target, symbolTable);
        pMips->addInstruction(new MIPS_Instruction(MIPS_MOVE, "$a0", num->getName()));
    }
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LA, "$a0", NEWLINE));

    auto *syscall = new MIPS_Instruction(MIPS_SYSCALL);
    syscall->setComments(pInst->toString());
    pMips->addInstruction(syscall);

}


void Block::generateBranch(Mips *pMips, IRInst *pInst) {
    int value;
    Reg *src1, *src2;
    if (isNumber(pInst->arg1, &value)) {
        src1 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src1->getName(), to_string(value));
        pMips->addInstruction(inst);
        src1->addr = allocator->CONSTANT;
    } else
        src1 = getRegOfSymbol(pInst->arg1);

    if (isNumber(pInst->arg2, &value)) {
        src2 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src2->getName(), to_string(value));
        pMips->addInstruction(inst);
        src2->addr = allocator->CONSTANT;
    } else
        src2 = getRegOfSymbol(pInst->arg2);

    increaseUse(pInst->arg1, symbolTable);
    increaseUse(pInst->arg2, symbolTable);

    MipsOperator branch;
    switch (pInst->irOperator) {
        case IR_IF_LT:
            branch = MIPS_BLT;
            break;
        case IR_IF_LE:
            branch = MIPS_BLE;
            break;
        case IR_IF_GT:
            branch = MIPS_BGT;
            break;
        case IR_IF_GE:
            branch = MIPS_BGE;
            break;
        case IR_IF_EQ:
            branch = MIPS_BEQ;
            break;
        case IR_IF_NE:
            branch = MIPS_BNE;
            break;
        default:
            break;
    }
    auto b = new MIPS_Instruction(branch, pInst->target, src1->getName(), src2->getName());
    pMips->addInstruction(b);
    b->setComments(pInst->toString());
    if (src1->addr == allocator->CONSTANT) {
        src1->addr = nullptr;
    }
    if (src2->addr == allocator->CONSTANT) {
        src2->addr = nullptr;
    }

}


Reg *Block::getRegOfSymbol(const string &varName) {
    auto item = this->symbolTable.find(varName);
    if (item != symbolTable.end()) {
        auto addr = item->second;
        if (!addr->reg) {
            Reg *reg = allocator->localAllocate(mips);
            addr->reg = reg;
            reg->addr = addr;
            int offset = addr->offset;
            // if it is in memory, load to reg, negative offset means first use
            if (offset > 0) {
                addr->loadToReg(mips);
            }
        }
        return addr->reg;
    }
    // for some variable will not used by follow inst
    Reg *reg = allocator->localAllocate(mips);
    symbolTable.insert(pair<string, AddressDescriptor *>(varName, new AddressDescriptor(varName, reg, -1)));
    return reg;
}

void addSymbolUsed(const string &var, int lineNo, map<string, AddressDescriptor *> &symbolTable) {
    if (var[0] == '#') {
        return;
    }
    auto item = symbolTable.find(var);
    if (item != symbolTable.end()) {
        item->second->setNextUsed(lineNo);
        return;
    }
    auto addr = new AddressDescriptor(var, nullptr, -1);
    symbolTable.insert(pair<string, AddressDescriptor *>(var, addr));
    addr->setNextUsed(lineNo);
}

void Block::analysis() {
    auto itor = this->end;
    int reverse_line = this->numOfInst;

    resetSymbolTable();
    if ((*this->start)->irOperator == IR_FUNCTION) {
        // I do not know how to release new memory in map
        this->symbolTable.clear();
        this->allocator->reset();
    }
    while (itor != this->start) {
        itor--;
        reverse_line--;
        IRInst *inst = *itor;
        switch (inst->irOperator) {

            case IR_ASSIGN:
                addSymbolUsed(inst->arg1, reverse_line, symbolTable);
                break;
            case IR_ADDRESS:
                break;
            case IR_GET_VALUE_IN_ADDRESS:
                break;
            case IR_COPY_VALUE_TO_ADDRESS:
                break;
            case IR_IF_LT:
            case IR_IF_LE:
            case IR_IF_GT:
            case IR_IF_GE:
            case IR_IF_EQ:
            case IR_IF_NE:
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV: {
                addSymbolUsed(inst->arg1, reverse_line, symbolTable);
                addSymbolUsed(inst->arg2, reverse_line, symbolTable);
            }
            case IR_RETURN:
                break;
            case IR_DEC:
                break;
            case IR_PARAM:
                this->parameters_of_function.push_front(inst);
                break;
            case IR_CALL:
                break;
            case IR_ARG:
            case IR_WRITE:
                addSymbolUsed(inst->target, reverse_line, symbolTable);
                break;
        }

    }
}

void Block::resetSymbolTable() const {
    for (const auto &item: symbolTable) {
        AddressDescriptor *addr = item.second;
        if (addr->reg) {
            addr->reg->reset();
            addr->reg = nullptr;
        }
        if (addr->hasNextUse()) {
            cerr << "why lie" << endl;
            exit(22);
        }
    }
}

void Block::setMips(Mips *pmips) {
    Block::mips = pmips;
}

Block::Block(RegisterAllocator *allocator,
             map<string, AddressDescriptor *> &symbolTable) : symbolTable(symbolTable) {
    this->allocator = allocator;
}

void Block::bindingArgumentRegister(Reg *arg, string &argName, int offset) const {
    auto item = this->symbolTable.find(argName);
    if (item != symbolTable.end()) {
        AddressDescriptor *addr = item->second;
        addr->reg = arg;
        arg->addr = addr;
        addr->forward = true;
        addr->offset = offset;
        addr->loadToReg(mips);
    } else {
        auto addr = new AddressDescriptor(argName, arg, offset);
        this->symbolTable.insert(pair<string, AddressDescriptor *>(argName, addr));
        addr->forward = true;
        addr->loadToReg(mips);
    }
}

inline void Block::saveRegisterStatus(Mips *pMips) const {
    for (auto &item: this->symbolTable) {
        AddressDescriptor *addr = item.second;
        if (!addr->reg || addr->isUseless())
            continue;
        if (addr->reg->isDirty()) {
            if (addr->offset < 0) {
                addr->offset = allocator->getSpace();
                mips->push(4);
            }
            addr->saveToMemory(pMips);
            addr->reg->removeDirty();
        }
    }
}


inline void Block::restoreRegisterStatus(Mips *pMips) {
    for (auto &item: this->symbolTable) {
        AddressDescriptor *addr = item.second;
        if (!addr->reg || addr->isUseless())
            continue;
        addr->loadToReg(pMips);
    }
}

void Block::saveRegisterArg0(Mips *pMips) {
    for (auto &item: this->symbolTable) {
        AddressDescriptor *addr = item.second;
        if (addr->reg && addr->reg->getName() == "$a0") {
            addr->saveToMemory(pMips);
            addr->reg->reset();
            addr->reg = nullptr;
        }
    }
}

