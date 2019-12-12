//
// Created by hya on 12/12/19.
//

#ifndef SPL_OPTIMIZER_H
#define SPL_OPTIMIZER_H

#include "irnode.h"

class Optimizer {

    map<string, int> referCount;
    map<string, string> localSymbleTable;

    void optimizerConstant(list<IRInst *> &insts, int max_depth);

    bool mergeInst(list<IRInst *> &insts, list<IRInst *>::iterator &itor);

    void addReferCount(list<IRInst *>::iterator &iterator);

    void rmReferCount(list<IRInst *>::iterator &iterator);

public:
    IR *optimize(IR *ir);

    static bool cacExpression(IRInst *inst, int *value);
};

#endif //SPL_OPTIMIZER_H
