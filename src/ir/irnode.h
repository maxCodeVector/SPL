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
    IR_GET_VALUE_IN_ADDRESS,
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


struct IRInst {
    IROperator irOperator;
    string target;
    string arg1;
    string arg2;

    string toString();

    IRInst(IROperator irOperator, const string &target, const string &arg1, const string &arg2);

    IRInst(IROperator irOperator, const string &target);

    friend ostream &operator<<(ostream &, IRInst &inst);
};

ostream &operator<<(ostream &os, IRInst *inst);

class IR {
protected:
    list<IR *> blocks;
protected:
    list<IRInst *> instructions;
public:
    void write(ostream &basicOstream);

    const list<IR *> &getBlocks() const;

    void addBlock(IR *ir);

    virtual list<IRInst *> *getInstructions() { return nullptr; }

    friend class Optimizer;
};

class IRExpr : public IR {
public:
    void addInstruction(IROperator irOperator, string &target);

    void addInstruction(IROperator irOperator, string &target, string &arg1, string &arg2);

    list<IRInst *> *getInstructions() override;


};


class IRStatement : public IR {
public:
    IRInst *addInstruction(IROperator irOperator, const string &target);

    void addInstruction(IRInst *inst);

    IRInst *addInstruction(IROperator irOperator, const string &target, const string &arg1, const string &arg2);

    list<IRInst *> *getInstructions() override;


};


class IRVisitor {
public:
    ErrorHandler errorHandler;

    virtual string getAddress(string &id) {
        return id;
    }

    virtual void handleBreakAndContinue(Operator bc, Location *location) = 0;

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
    string labelName;
    list<IRInst *> jumpInst;
public:
    const list<IRInst *> &getJumpInst() const;

public:
    explicit JumpEntry(const string &labelName);

    void addInst(IRInst *inst);

    void removeInst(IRInst *inst);
};

bool isSimilarGoto(IROperator irOperator);

#endif //SPL_IRNODE_H
