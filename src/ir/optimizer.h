//
// Created by hya on 12/12/19.
//

#ifndef SPL_OPTIMIZER_H
#define SPL_OPTIMIZER_H

#include "irnode.h"

bool isNumber(const string &arg, int *value);

class Optimizer {

    map<string, int> referCount;
    map<string, string> localSymbleTable;

    void optimizerConstant(list<IRInst *> &insts, int max_depth);

    bool mergeInst(list<IRInst *> &insts);

    void addReferCount(list<IRInst *>::iterator &iterator);

    void rmReferCount(list<IRInst *>::iterator &iterator);

public:
    IR *optimize(IR *ir);

    /**
     *
     * @param inst
     * @param flag 1 means optimized half, 2 means optimized to number,
     *             0 not optimized, -1 will be runtime error (divide by 0)
     * @return
     */
    static IRInst *cacExpression(IRInst *inst, int *flag);
};

#endif //SPL_OPTIMIZER_H
