//
// Created by hya on 12/17/19.
//

#ifndef SPL_CODE_GENERATOR_H
#define SPL_CODE_GENERATOR_H

#include "../ir/irnode.h"
#include "mips.h"

class CodeGenerator {
public:
    explicit CodeGenerator(IR* ir);

    Mips* generateMipsCode();

};


#endif //SPL_CODE_GENERATOR_H
