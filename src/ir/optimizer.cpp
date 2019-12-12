//
// Created by hya on 12/12/19.
//

#include "optimizer.h"

bool isNumber(const string &arg, int *value) {
    if (arg.empty())
        return false;
    if (arg[0] == '#') {
        *value = atoi(arg.substr(1, arg.size() - 1).c_str());
        return true;
    }
    return false;
}

bool isVar(const string &arg, char flag) {
    if (arg.empty())
        return false;
    return arg[0] == flag;
}

bool isSimilarAssignOp(IROperator irOperator) {
    return irOperator == IR_ADD || irOperator == IR_SUB
           || irOperator == IR_MUL || irOperator == IR_DIV;
}

bool Optimizer::mergeInst(list<IRInst *> &insts, list<IRInst *>::iterator &itor) {
    if (itor == insts.begin())
        return false;
    auto pre = itor;
    pre--;
    IRInst *curr = *itor;
    if (curr->irOperator == IR_ASSIGN) {
        IRInst *preInst = *pre;
        if (isSimilarAssignOp(preInst->irOperator) && preInst->target == curr->arg1) {
            if (isVar(preInst->target, 't')) {
                preInst->target = curr->target;
                return true;
            }
        }
    }
    return false;
}


IR *Optimizer::optimize(IR *ir) {
//    return ir;
    if (!ir)
        return ir;
//    optimizerConstant(ir->instructions, 2);

    auto iterator = ir->instructions.begin();
    while (iterator != ir->instructions.end()) {
        if (mergeInst(ir->instructions, iterator)) {
            auto inst = *iterator;
            iterator = ir->instructions.erase(iterator);
            delete (inst);
        } else
            iterator++;
    }
    return ir;
}

void putSymbol(map<string, int> &symbol, string &key) {
    auto item = symbol.find(key);
    if (item != symbol.end()) {
        item->second++;
    } else
        symbol.insert(pair<string, int>(key, 1));
}

void Optimizer::optimizerConstant(list<IRInst *> &insts, int max_depth = 3) {
    auto iterator = insts.begin();
    while (iterator != insts.end()) {
        list<IRInst *> localInsts;
        auto curr = iterator;
        for (int i = 0; i < max_depth; i++) {
            localInsts.push_back(*curr);
            if (curr == insts.begin()) {
                break;
            }
            curr--;
        }
        map<string, int> leftSymbol;
        map<string, int> rightSymbol;
        for (IRInst *inst:localInsts) {
            int terminal_flag = 0;
            switch (inst->irOperator) {
                case IR_ASSIGN: {
                    putSymbol(leftSymbol, inst->target);
                    putSymbol(rightSymbol, inst->arg1);
                }
                    break;
                case IR_ADD: {
                    putSymbol(leftSymbol, inst->target);
                    putSymbol(rightSymbol, inst->arg1);
                    putSymbol(rightSymbol, inst->arg2);
                }
                    break;
                case IR_SUB: {
                    putSymbol(leftSymbol, inst->target);
                    putSymbol(rightSymbol, inst->arg1);
                    putSymbol(leftSymbol, inst->arg2);
                }
                    break;
                default:
                    terminal_flag = 1;
            }
            if (terminal_flag)
                break;
        }
        auto item = leftSymbol.begin();
        while (item != leftSymbol.end()) {
            auto rightItem = rightSymbol.find(item->first);
            if (rightItem != rightSymbol.end()) {
                rightItem->second--;
                if (rightItem->second == 0) {
                    rightSymbol.erase(item->first);
                }
                item->second--;
            }
            if (item->second == 0) {
                item = leftSymbol.erase(item);
            } else {
                item++;
            }
        }
        // means it has chance to merge
        if (leftSymbol.size() == 1 && leftSymbol.begin()->second == 1) {
            IRInst* currInst = *iterator;
            switch (rightSymbol.size()) {
                case 1:
                    currInst->arg1 = rightSymbol.begin()->first;
                    currInst->irOperator = IR_ASSIGN;
                    iterator--;
                    iterator = insts.erase(iterator);
                    iterator++;
                    continue;
                case 2:
                default:
                    break;
            }

        }
        iterator++;
    }
}

bool Optimizer::cacExpression(IRInst *inst, int *value) {
    if (!isSimilarAssignOp(inst->irOperator))
        return false;
    if (inst->irOperator == IR_SUB && inst->arg1 == inst->arg2) {
        *value = 0;
        return true;
    }
    int number1, number2;
    if (isNumber(inst->arg1, &number1) && isNumber(inst->arg2, &number2)) {
        int res;
        switch (inst->irOperator) {
            case IR_ADD:
                res = number1 + number2;
                break;
            case IR_SUB:
                res = number1 - number2;
                break;
            case IR_MUL:
                res = number1 * number2;
                break;
            case IR_DIV:
                res = number1 / number2;
                break;
            default:
                res = 0;
        }
        *value = res;
        return true;
    }
    return false;
}

void Optimizer::addReferCount(list<IRInst *>::iterator &iterator) {
    auto inst = *iterator;
    if (!inst->arg1.empty()) {
        auto item = this->referCount.find(inst->arg1);
        if (item == referCount.end()) {
            referCount.insert(pair<string, int>(inst->arg1, 1));
        } else
            (item->second)++;
    }
    if (!inst->arg2.empty()) {
        auto item = this->referCount.find(inst->arg2);
        if (item == referCount.end()) {
            referCount.insert(pair<string, int>(inst->arg2, 1));
        } else
            (item->second)++;
    }
}

void Optimizer::rmReferCount(list<IRInst *>::iterator &iterator) {
    auto inst = *iterator;
    if (!inst->arg1.empty()) {
        auto item = this->referCount.find(inst->arg1);
        if (item != referCount.end()) {
            (item->second)--;
        }
    }
    if (!inst->arg2.empty()) {
        auto item = this->referCount.find(inst->arg2);
        if (item != referCount.end()) {
            (item->second)--;
        }
    }
}

