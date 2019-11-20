//
// Created by hya on 11/19/19.
//

#ifndef SPL_TYPE_H
#define SPL_TYPE_H

#include "ast.h"
#include "error.h"

class DereferenceChecker : Visitor{
public:
    void resolve(AST& ast) override;
    void resolve(Body& body) override;
    explicit DereferenceChecker(ErrorHandler& errorHandle);
};


#endif //SPL_TYPE_H
