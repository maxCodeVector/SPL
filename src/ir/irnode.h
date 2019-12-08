//
// Created by hya on 12/8/19.
//

#ifndef SPL_IRNODE_H
#define SPL_IRNODE_H

#include "../error.h"
#include "../ast.h"

enum IROperator {
    IR_LABEL,
    IR_FUNCTION,
    IR_ASSIGN,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DEV,
    IR_ADDRESS,
    IR_ASSIGN_VALUE_IN_ADDRESS,
    IR_COPY_VALUE_TO_ADDRESS,
    IR_GOTO,
    IR_RELOP_GOTO,
    IR_RETURN,
    IR_DEC,
    IR_PARAM,
    IR_ARG,
    IR_CALL,
    IR_READ,
    IR_WRITE
};

class IRInst {
public:
    IROperator irOperator;
    string target;
    string arg1;
    string arg2;
};

class IRExpr : public IRInst {


};

class IRStatement : public IRInst {

};


class IRVisitor {
public:
    ErrorHandler errorHandler;

    virtual void visit(Exp *expNode) = 0;

    virtual void visit(Statement *statementNode) = 0;

};

class JumpEntry {

};

class IR {
public:
    void write(ostream &basicOstream);

};

#endif //SPL_IRNODE_H
