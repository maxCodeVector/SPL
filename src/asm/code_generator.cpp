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
    mips = new Mips;
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
    auto itor = start;
    const auto &temp = getAllIRcode();
    while (itor != end) {
        generateCode(*itor);
        itor++;
    }
}

void Block::generateCode(IRInst *inst) {
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
            generateParameter(mips, inst);
            break;
        case IR_ARG:
            generateArgument(mips, inst);
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

void Block::generateArgument(Mips *pMips, IRInst *pInst) {
    this->arguments_of_next_call.push_back(pInst);
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
        } else
            arg = getRegOfSymbol(inst->target);
        pMips->addInstruction(new MIPS_Instruction(MIPS_SW, arg->getName(), "$sp", to_string(arg_count * 4)));
        arg_count++;
    }
    this->arguments_of_next_call.clear();


    pMips->addInstruction(new MIPS_Instruction(MIPS_JAL, pInst->arg1));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LW, "$ra", "$fp", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LW, "$fp", "$fp", "0"));

    /** restore register value form memory*/
    restoreRegisterStatus(mips);
    Reg *ret = getRegOfSymbol(pInst->target);
    pMips->addInstruction(new MIPS_Instruction(MIPS_MOVE, ret->getName(), "$v0"));

    pMips->pop(4 * numOfArgs + 8);
}


void Block::generateParameter(Mips *pMips, IRInst *pInst) {
    // useless
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
    auto jr = new MIPS_Instruction(MIPS_JR, "$ra");
    pMips->addInstruction(jr);
}


void Block::generateAssign(Mips *mips, const IRInst *inst) {
    Reg *dest = getRegOfSymbol(inst->target);
    int value;
    if (isNumber(inst->arg1, &value)) {
        auto *mipsInst = new MIPS_Instruction(MIPS_LI, dest->getName(), to_string(value));
        mips->addInstruction(mipsInst);
    } else {
        Reg *src = getRegOfSymbol(inst->arg1);
        src->addr->use();
        auto *mipsInst = new MIPS_Instruction(MIPS_MOVE, dest->getName(), src->getName());
        mips->addInstruction(mipsInst);
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
    } else
        src1 = getRegOfSymbol(pInst->arg1);

    if (isNumber(pInst->arg2, &value)) {
        src2 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src2->getName(), to_string(value));
        pMips->addInstruction(inst);
    } else
        src2 = getRegOfSymbol(pInst->arg2);

    Reg *dest = getRegOfSymbol(pInst->target);
    switch (pInst->irOperator) {
        case IR_ADD: {
            auto add = new MIPS_Instruction(MIPS_ADD, dest->getName(), src1->getName(), src2->getName());
            pMips->addInstruction(add);
        }
            break;
        case IR_SUB: {
            auto sub = new MIPS_Instruction(MIPS_SUB, dest->getName(), src1->getName(), src2->getName());
            pMips->addInstruction(sub);
        }
            break;
        case IR_MUL: {
            auto mul = new MIPS_Instruction(MIPS_MUL, dest->getName(), src1->getName(), src2->getName());
            pMips->addInstruction(mul);
        }
            break;
        case IR_DIV:
            break;
        default:
            break;
    }
    increaseUse(pInst->arg1, symbolTable);
    increaseUse(pInst->arg2, symbolTable);
    dest->setDirty();
}

void Block::generateRead(Mips *pMips, IRInst *pInst) {
    saveRegisterStatus(mips);

    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LA, "$a0", INPUT_HINT));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));

    restoreRegisterStatus(mips);

    Reg *num = getRegOfSymbol(pInst->target);
    increaseUse(pInst->target, symbolTable);
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "5"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));
    pMips->addInstruction(new MIPS_Instruction(MIPS_MOVE, num->getName(), "$v0"));
    num->setDirty();

}

void Block::generateWrite(Mips *pMips, IRInst *pInst) {
    saveRegisterStatus(mips);

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
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));

    restoreRegisterStatus(mips);

}


void Block::generateBranch(Mips *pMips, IRInst *pInst) {

    int value;
    Reg *src1, *src2;
    if (isNumber(pInst->arg1, &value)) {
        src1 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src1->getName(), to_string(value));
        pMips->addInstruction(inst);
    } else
        src1 = getRegOfSymbol(pInst->arg1);

    if (isNumber(pInst->arg2, &value)) {
        src2 = allocator->localAllocate(mips);
        auto inst = new MIPS_Instruction(MIPS_LI, src2->getName(), to_string(value));
        pMips->addInstruction(inst);
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
    pMips->addInstruction(
            new MIPS_Instruction(branch, pInst->target, src1->getName(), src2->getName()));
}


Reg *Block::getRegOfSymbol(const string &varName) {
    auto item = this->symbolTable.find(varName);
    // var has been in the register or memory
    if (item != symbolTable.end()) {
        auto addr = item->second;
        if (!addr->reg) {
            // load from memory
            Reg *reg = allocator->localAllocate(mips);
            addr->reg = reg;
            reg->addr = addr;
            int offset = addr->offset;
            if (offset > 0) {
                addr->loadToReg(mips);
            }
        }
        return addr->reg;
    }
    // first used for this variable
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

    if ((*this->start)->irOperator == IR_FUNCTION) {
        // I do not know how to release new memory in map
        symbolTable.clear();
        allocator->reset();
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
                addSymbolUsed(inst->target, reverse_line, symbolTable);
                this->parameters_of_function.push_front(inst);
                break;
            case IR_CALL:
                break;
            case IR_ARG:
            case IR_READ:
            case IR_WRITE:
                addSymbolUsed(inst->target, reverse_line, symbolTable);
                break;
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
        if (!addr->reg)
            continue;
        if (addr->offset < 0) {
            addr->offset = allocator->getSpace();
            mips->push(4);
        }
        addr->saveToMemory(pMips);
    }
}


inline void Block::restoreRegisterStatus(Mips *pMips) {
    for (auto &item: this->symbolTable) {
        AddressDescriptor *addr = item.second;
        if (!addr->reg)
            continue;
        addr->loadToReg(pMips);
    }
}

