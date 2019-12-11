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
    IR_DIV,
    IR_ADDRESS,
    IR_ASSIGN_VALUE_IN_ADDRESS,
    IR_COPY_VALUE_TO_ADDRESS,
    IR_GOTO,
    IR_IF_LT,
    IR_IF_LE,
    IR_IF_GT,
    IR_IF_GE,
    IR_IF_EQ,
    IR_IF_NE,
    IR_RETURN,
    IR_DEC,
    IR_PARAM,
    IR_ARG,
    IR_CALL,
    IR_READ,
    IR_WRITE
};

class IRInst {
    IROperator irOperator;
    string target;
    string arg1;
    string arg2;

    string toString();

public:

    IRInst(IROperator irOperator, const string &target, const string &arg1, const string &arg2);

    IRInst(IROperator irOperator, const string &target);

    friend ostream &operator<<(ostream &, IRInst &inst);
};

ostream &operator<<(ostream &os, IRInst *inst);

class IR {
    list<IR *> blocks;
public:
    void write(ostream &basicOstream);

    void addBlock(IR *ir);

    virtual list<IRInst *> *getInstructions() { return nullptr; }


};

class IRExpr : public IR {
    list<IRInst *> instructions;
public:
    void addInstruction(IROperator irOperator, string &target);

    void addInstruction(IROperator irOperator, string &target, string &arg1, string &arg2);

    list<IRInst *> *getInstructions() override;


};


class IRStatement : public IR {
    list<IRInst *> instructions;
public:
    void addInstruction(IROperator irOperator, const string &target);

    void addInstruction(IROperator irOperator, const string &target, const string &arg1, const string &arg2);

    list<IRInst *> *getInstructions() override;


};


class IRVisitor {
public:
    ErrorHandler errorHandler;

    virtual void visit(BinaryExp *expNode) = 0;

    virtual void visit(UnaryExp *expNode) = 0;

    virtual void visit(InvokeExp *expNode) = 0;

    virtual void visit(GetAttributeExp *expNode) = 0;

    virtual void visit(Variable *variable) = 0;

    virtual void visit(ReturnStatement *statementNode) = 0;

    virtual void visit(IfStatement *statementNode) = 0;

    virtual void visit(WhileStatement *statementNode) = 0;

};

class JumpEntry {

};


#endif //SPL_IRNODE_H
