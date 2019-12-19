//
// Created by hya on 12/17/19.
//

#include "code_generator.h"
#include "../ir/optimizer.h"

CodeGenerator::CodeGenerator(IR *ir) {
    for (auto block: ir->getBlocks()) {
        for (IRInst *inst: *block->getInstructions()) {
            this->ir_code.push_back(inst);
        }
    }
}


Mips *CodeGenerator::generateMipsCode() {
    Mips *mips = new Mips;
    auto iter = this->ir_code.begin();
    while (iter != this->ir_code.end()) {
        generateCode(mips, *iter);
        iter++;
    }
    return mips;
}

Reg *CodeGenerator::getRegOfSymbol(const string &varName) {
    auto item = this->symbolTable.find(varName);
    if (item != symbolTable.end()) {
        return item->second;
    }
    //todo load from memory
    Reg *reg = allocator.allocate();
    symbolTable.insert(pair<string, Reg *>(varName, reg));
    return reg;
}

void CodeGenerator::generateCode(Mips *mips, IRInst *inst) {
    switch (inst->irOperator) {

        case IR_LABEL:
            break;
        case IR_FUNCTION: {
            auto *mipsInstruction = new MIPS_Instruction{MIPS_LABEL, inst->target};
            mips->addInstruction(mipsInstruction);
        }
            break;
        case IR_ASSIGN:
            generateAssign(mips, inst);
            break;
        case IR_ADD:
        case IR_SUB:
            generateAddSub(mips, inst);
            break;
        case IR_MUL:
            generateMultiply(mips, inst);
            break;
        case IR_DIV:
            break;
        case IR_ADDRESS:
            break;
        case IR_GET_VALUE_IN_ADDRESS:
            break;
        case IR_COPY_VALUE_TO_ADDRESS:
            break;
        case IR_GOTO:
            break;
        case IR_IF_LT:
            break;
        case IR_IF_LE:
            break;
        case IR_IF_GT:
            break;
        case IR_IF_GE:
            break;
        case IR_IF_EQ:
            break;
        case IR_IF_NE:
            break;
        case IR_RETURN:
            generateReturn(mips, inst);
            break;
        case IR_DEC:
            break;
        case IR_PARAM:
            break;
        case IR_ARG:
            break;
        case IR_CALL:
            break;
        case IR_READ:
            generateRead(mips, inst);
            break;
        case IR_WRITE:
            generateWrite(mips, inst);
            break;
    }

}

void CodeGenerator::generateAssign(Mips *mips, const IRInst *inst) {
    Reg *dest = getRegOfSymbol(inst->target);
    int value;
    if (isNumber(inst->arg1, &value)) {
        auto *mipsInst = new MIPS_Instruction(MIPS_LI, dest->getName(), to_string(value));
        mips->addInstruction(mipsInst);
    } else {
        Reg *src = getRegOfSymbol(inst->arg1);
        auto *mipsInst = new MIPS_Instruction(MIPS_MOVE, dest->getName(), src->getName());
        mips->addInstruction(mipsInst);
    }
}


void CodeGenerator::generateAddSub(Mips *pMips, IRInst *pInst) {
    Reg *src1 = getRegOfSymbol(pInst->arg1);
    Reg *src2 = getRegOfSymbol(pInst->arg2);
    Reg *dest = getRegOfSymbol(pInst->target);
    if (pInst->irOperator == IR_ADD) {
        auto mips = new MIPS_Instruction(MIPS_ADD, dest->getName(), src1->getName(), src2->getName());
        pMips->addInstruction(mips);
    } else {
        auto mips = new MIPS_Instruction(MIPS_SUB, dest->getName(), src1->getName(), src2->getName());
        pMips->addInstruction(mips);
    }

}

void CodeGenerator::generateProcedure() {

}

void CodeGenerator::generateMultiply(Mips *pMips, IRInst *pInst) {
    int value;
    Reg *src1, *src2;
    // todo add zero register optimize
    if (isNumber(pInst->arg1, &value)) {
        src1 = allocator.allocate();
        auto inst = new MIPS_Instruction(MIPS_LI, src1->getName(), to_string(value));
        pMips->addInstruction(inst);
    } else
        src1 = getRegOfSymbol(pInst->arg1);

    if (isNumber(pInst->arg2, &value)) {
        src2 = allocator.allocate();
        auto inst = new MIPS_Instruction(MIPS_LI, src2->getName(), to_string(value));
        pMips->addInstruction(inst);
    } else
        src2 = getRegOfSymbol(pInst->arg2);
    Reg *dest = getRegOfSymbol(pInst->target);
    auto mul = new MIPS_Instruction(MIPS_MUL, dest->getName(), src1->getName(), src2->getName());
    pMips->addInstruction(mul);
}

void CodeGenerator::generateRead(Mips *pMips, IRInst *pInst) {
    Reg *num = getRegOfSymbol(pInst->target);
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LA, "$a0", INPUT_HINT));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "5"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));
    pMips->addInstruction(new MIPS_Instruction(MIPS_MOVE, num->getName(), "$v0"));
}

void CodeGenerator::generateWrite(Mips *pMips, IRInst *pInst) {
    Reg *num = getRegOfSymbol(pInst->target);
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "1"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_MOVE, "$a0", num->getName()));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LI, "$v0", "4"));
    pMips->addInstruction(new MIPS_Instruction(MIPS_LA, "$a0", NEWLINE));
    pMips->addInstruction(new MIPS_Instruction(MIPS_SYSCALL));
}

void CodeGenerator::generateReturn(Mips *pMips, IRInst *pInst) {
    int value;
    Reg *src;
    if (isNumber(pInst->target, &value)) {
        auto move = new MIPS_Instruction(MIPS_LI, "$v0", to_string(value));
        pMips->addInstruction(move);
    } else {
        src = getRegOfSymbol(pInst->target);
        auto move = new MIPS_Instruction(MIPS_MOVE, "$v0", src->getName());
        pMips->addInstruction(move);
    }
    auto jr = new MIPS_Instruction(MIPS_JR, "$ra");
    pMips->addInstruction(jr);
}

Reg *RegisterAllocator::allocate() {
    for (Reg &reg: temp_regs) {
        if (!reg.dirty) {
            reg.dirty = true;
            return &reg;
        }
    }
    return &temp_regs[0];
}


RegisterAllocator::RegisterAllocator() {
    for (int i = 0; i < reg_number; i++) {
        temp_regs[i].id = i;
        temp_regs[i].dirty = false;
    }
}

string Reg::getName() {
    return "$t" + to_string(id);
}
