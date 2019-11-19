//
// Created by hya on 11/19/19.
//

#ifndef SPL_TYPE_H
#define SPL_TYPE_H

#include "ast.h"

class TypeResolver :Visitor{
public:
    virtual void resolve(AST& ast)=0;
    virtual void resolve(Body& body)=0;
    virtual void resolve(Statement& statement)=0;
};


#endif //SPL_TYPE_H
