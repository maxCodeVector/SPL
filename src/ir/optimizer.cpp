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

IR *Optimizer::optimize(IR *ir) {
//    return ir;
    if (!ir)
        return ir;
    optimizerConstant(ir->instructions, 2);
    mergeInst(ir->instructions);
    return ir;
}

bool Optimizer::mergeInst(list<IRInst *> &insts) {
    auto itor = insts.begin();
    while (itor != insts.end()) {
        if (itor == insts.begin()) {
            itor++;
            continue;
        }
        auto pre = itor;
        pre--;
        IRInst *preInst = *pre;
        IRInst *curr = *itor;
        if (curr->irOperator == IR_ASSIGN) {
            // case need to delete current instruction
            if (preInst->target == curr->arg1 && (isSimilarAssignOp(preInst->irOperator)
                                                  || preInst->irOperator == IR_GET_VALUE_IN_ADDRESS
                                                  || preInst->irOperator == IR_CALL
                                                  || preInst->irOperator == IR_READ)
                    ) {
                if (isVar(preInst->target, 't')) {
                    preInst->target = curr->target;
                    itor = insts.erase(itor);
                    delete (curr);
                }
            }
        } else if (preInst->irOperator == IR_ASSIGN) {
            // case need to delete pre instruction
            if (isVar(preInst->target, 't')) {
                int replace_flag = 0;
                if (curr->arg1 == preInst->target) {
                    curr->arg1 = preInst->arg1;
                    replace_flag = 1;
                }
                if (curr->arg2 == preInst->target) {
                    curr->arg2 = preInst->arg1;
                    replace_flag = 1;
                }
                if (curr->target == preInst->target) {
                    curr->target = preInst->arg1;
                    replace_flag = 1;
                }
                if (replace_flag) {
                    itor = insts.erase(pre);
                    delete (preInst);
                }
            }
        }
        itor++;
    }

    return false;
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
            IRInst *currInst = *curr;
            if (currInst->irOperator != IR_ASSIGN &&
                currInst->irOperator != IR_ADD &&
                currInst->irOperator != IR_SUB) {
                break;
            }
            localInsts.push_back(currInst);
            if (curr == insts.begin()) {
                break;
            }
            curr--;
        }
        if (localInsts.size() != max_depth) {
            iterator++;
            continue;
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
        IRInst *currInst = *iterator;
        auto item = leftSymbol.begin();
        while (item != leftSymbol.end()) {
            if (item->first == currInst->target) {
                item++;
                continue;
            }
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
        list<string> posSymbol;
        list<string> negSymbol;
        int constant1 = 0;
        for (auto &left:leftSymbol) {
            if (left.first != currInst->target) {
                int value;
                if (isNumber(left.first, &value)) {
                    constant1 += value * left.second;
                } else {
                    for (int i = 0; i < left.second; i++) {
                        negSymbol.push_back(left.first);
                    }
                }
            }
        }
        if (constant1 != 0) {
            negSymbol.push_back("#" + to_string(constant1));
        }

        int constant2 = 0;
        for (auto &right:rightSymbol) {
            int value;
            if (isNumber(right.first, &value)) {
                constant2 += value * right.second;
            } else
                for (int i = 0; i < right.second; i++) {
                    posSymbol.push_back(right.first);
                }
        }
        if (constant2 != 0) {
            posSymbol.push_back("#" + to_string(constant2));
        }

        int posSymbolNum = posSymbol.size();
        int negSymbolNum = negSymbol.size();
        if (posSymbolNum + negSymbolNum <= 2) {
            curr = iterator;
            curr--;
            IRInst *preInst = *curr;
            if (isVar(preInst->target, 't')) {
                if (posSymbolNum == 2) {
                    currInst->arg1 = posSymbol.front();
                    currInst->arg2 = posSymbol.back();
                    currInst->irOperator = IR_ADD;
                } else if (posSymbolNum == 1 && negSymbolNum == 0) {
                    currInst->arg1 = posSymbol.front();
                    currInst->irOperator = IR_ASSIGN;
                } else if (posSymbolNum == 1 && negSymbolNum == 1) {
                    currInst->arg1 = posSymbol.front();
                    currInst->arg2 = negSymbol.front();
                    currInst->irOperator = IR_SUB;
                } else if (negSymbolNum == 0) {
                    exit(-3);
                } else if (negSymbolNum == 1) {
                    exit(-3);
                } else if (negSymbolNum == 2) {
                    exit(-3);
                }
                int flag;
                cacExpression(currInst, &flag);
                iterator = insts.erase(curr);
                delete (preInst);
            }
        }
        iterator++;
    }
}

IRInst *Optimizer::cacExpression(IRInst *inst, int *flag) {
    if (!isSimilarAssignOp(inst->irOperator)) {
        *flag = 0;
        return nullptr;
    }
    if (inst->arg1 == inst->arg2) {
        if (inst->irOperator == IR_SUB) {
            *flag = 2;
            inst->arg1 = "#0";
            inst->irOperator = IR_ASSIGN;
            return inst;
        }
        if (inst->irOperator == IR_DIV) {
            *flag = 2;
            inst->arg1 = "#1";
            inst->irOperator = IR_ASSIGN;
            return inst;
        }
    }
    int number1, number2;
    bool isNum1 = isNumber(inst->arg1, &number1);
    bool isNum2 = isNumber(inst->arg2, &number2);
    if (isNum1 && isNum2) {
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
        *flag = 2;
        inst->arg1 = "#" + to_string(res);
        inst->irOperator = IR_ASSIGN;
        return inst;
    } else if (isNum1) {
        switch (inst->irOperator) {
            case IR_ADD:
            case IR_SUB:
                if (number1 == 0) {
                    *flag = 1;
                    inst->arg1 = inst->arg2;
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                break;
            case IR_MUL:
                if (number1 == 0) {
                    *flag = 2;
                    inst->arg1 = "#0";
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                if (number1 == 1) {
                    *flag = 1;
                    inst->arg1 = inst->arg2;
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                break;
            case IR_DIV:
                if (number1 == 0) {
                    *flag = 2;
                    inst->arg1 = "#0";
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                break;
            default:
                break;
        }
    } else if (isNum2) {
        switch (inst->irOperator) {
            case IR_ADD:
            case IR_SUB:
                if (number2 == 0) {
                    *flag = 1;
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                break;
            case IR_MUL:
                if (number2 == 0) {
                    *flag = 2;
                    inst->arg1 = "#0";
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                if (number2 == 1) {
                    *flag = 1;
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                break;
            case IR_DIV:
                if (number2 == 1) {
                    *flag = 1;
                    inst->irOperator = IR_ASSIGN;
                    return inst;
                }
                if (number2 == 0) {
                    *flag = -1;
                    return inst;
                }
                break;
            default:
                break;
        }
    }
    *flag = 0;
    return nullptr;
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

