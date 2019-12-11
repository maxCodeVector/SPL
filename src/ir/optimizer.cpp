//
// Created by hya on 12/12/19.
//

#include "optimizer.h"

void optimize(IR *ir);

bool isNumber(const string &arg, int *value) {
    if (arg.empty())
        return false;
    if (arg[0] == '#') {
        *value = atoi(arg.substr(1, arg.size() - 1).c_str());
        return true;
    }
    return false;
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
            auto item = this->referCount.find(curr->arg1);
            if(item==referCount.end()||item->second<=0){
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
    auto iterator = ir->instructions.begin();
    while (iterator != ir->instructions.end()) {
        addReferCount(iterator);
        iterator++;
    }

    iterator = ir->instructions.begin();
    while (iterator != ir->instructions.end()) {
        optimizerConstant(ir->instructions, iterator, 1);
        iterator++;
    }

    iterator = ir->instructions.begin();
    while (iterator != ir->instructions.end()) {
        rmReferCount(iterator);
        if (mergeInst(ir->instructions, iterator)) {
            iterator = ir->instructions.erase(iterator);
        } else
            iterator++;
    }
    return ir;
}

void Optimizer::optimizerConstant(list<IRInst *> &insts, list<IRInst *>::iterator &itor, int max_depth) {
    IRInst *inst = *itor;
    if(!isSimilarAssignOp(inst->irOperator))
        return;
    if(inst->arg1==inst->arg2) {
        inst->arg1 = "#0";
        inst->irOperator = IR_ASSIGN;
        return;
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
        inst->irOperator = IR_ASSIGN;
        inst->arg1 = "#" + to_string(res);
    }

}

void Optimizer::addReferCount(list<IRInst *>::iterator& iterator) {
    auto inst = *iterator;
    if(!inst->arg1.empty()){
        auto item = this->referCount.find(inst->arg1);
        if(item==referCount.end()){
            referCount.insert(pair<string,int >(inst->arg1, 1));
        } else
            (item->second) ++;
    }
    if(!inst->arg2.empty()){
        auto item = this->referCount.find(inst->arg2);
        if(item==referCount.end()){
            referCount.insert(pair<string,int >(inst->arg2, 1));
        } else
            (item->second) ++;
    }
}

void Optimizer::rmReferCount(list<IRInst *>::iterator &iterator) {
    auto inst = *iterator;
    if(!inst->arg1.empty()){
        auto item = this->referCount.find(inst->arg1);
        if(item!=referCount.end()){
            (item->second) --;
        }
    }
    if(!inst->arg2.empty()){
        auto item = this->referCount.find(inst->arg2);
        if(item!=referCount.end()){
            (item->second) --;
        }
    }
}

