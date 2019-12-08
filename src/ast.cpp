#include "error.h"
#include "semantic/scope.h"
#include "ast.h"
#include "ir/irnode.h"

void AST::findEntity(BaseNode *extList) {
    BaseNode *next = extList;
    while (next != nullptr) {
        if (next->flag == FUNC) {
            auto func = (Function *) next;
            this->functions.push_back(func);
        } else if (next->flag == VAR) {
            auto var = (Variable *) next;
            this->vars.push_back(var);
        } else if (next->flag == DECLARATION) {
            auto dec = (DeclaredVariableType *) next;
            string &decName = dec->getName();
            if ("baseType" != decName) {
                this->declaredTypes.push_back(dec->getType());
            } else
                delete (dec);
        }
        next = next->next;
    }
}

void AST::convert2AST(AttrNode *root) {
    findEntity(root->baseNode);
}

void AST::setScope(Scope *scope_) {
    this->toplevelScope = (ToplevelScope *) scope_;
}

void AST::setTypeTable(TypeTable *table) {
    typeTable = table;
}

list<Entity *> &AST::declaritions(list<Entity *> &decaries) {
    for (Variable *var: this->vars) {
        decaries.push_back(var);
    }
    for (Function *function: this->functions) {
        decaries.push_back(function);
    }
    return decaries;
}

list<Function *> &AST::getFunctions() {
    return this->functions;
}

void AST::setConstant(ConstantTable &constantTable) {

}

AST::~AST() {
    free_all(vars);
    free_all(functions);
    free_all(declaredTypes);
    delete (toplevelScope);
    delete (typeTable);
}

IR *AST::getIR() {
    for (Function *f: functions) {
        IR* block = f->getIr();
        if(block)
            ir->addBlock(block);
    }
    return ir;
}

AST::AST(AttrNode *root) {
    convert2AST(root);
    ir = new IR;
}




