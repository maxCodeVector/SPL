//
// Created by hya on 11/22/19.
//

#include "resolver.h"


void LocalResolver::resolve(AST &ast) {
    ToplevelScope *toplevelScope = new ToplevelScope();
    scopeStack.push_back(toplevelScope);

    list<Entity *> entities;
    for (Entity *e: ast.declaritions(entities)) {
        Entity *hasE = toplevelScope->declareEntity(*e);
        if (hasE != nullptr) {
            string message =
                    "duplicated define for:" + e->getName() + ", last defined in:" + hasE->getLocation()->toString();
            error(e->getLocation(), message);
        }
    }

    resolveGloableVarIntializers();
    resolveConstantValues();
    resolveDeclaredType(ast.getDeclaredTypes());
    resolveFunctions(ast.defineFunctions());
//    toplevelScope->checkReferences(this->errorHandler);
//        if(errorHandler.errorOccured()){
//            return;
//        }
    ast.setScope(toplevelScope);
    ast.setConstant(this->constantTable);
}

void LocalResolver::resolve(Body &body) {
    LocalScope *curr = (LocalScope *) currentScope();
    for (DefinedVariable *var: body.vars) {
        if (curr->isDefinedLocally(var->getName())) {
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), message);
        } else {
            curr->defineVariable(*var);
        }
    }
    for (DefinedVariable *var: body.vars) {
        if (var->getValue()) {
            error(var->getValue()->checkReference(currentScope()));
        }
    }
    for (Statement *statement:body.statements) {
        resolve(*statement);
    }
}

void LocalResolver::pushScope(list<DefinedVariable *> &vars) {
    Scope *parent = currentScope();
    LocalScope *scope = new LocalScope(parent);
    parent->children.emplace_back(scope);

    for (DefinedVariable *var: vars) {
        if (scope->isDefinedLocally(var->getName())) {
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), message);
        } else {
            scope->defineVariable(*var);
        }
    }
    scopeStack.push_back(scope);
}

void LocalResolver::resolveDeclaredType(list<VariableType *> &declared) {
    for (VariableType *variableType: declared) {
        if(variableType->getType()!=STRUCT_TYPE)
            continue;
        // unique member

        typeTable->declareVariableType(variableType, errorHandler);
    }
}

void LocalResolver::resolve(Statement &statement) {
    statement.checkReference(this, currentScope());
}

void LocalResolver::resolveFunctions(list<DefinedFunction *> &funcs) {
    for (DefinedFunction *function: funcs) {
        pushScope(function->getParameters());
        resolve(*function->getBody());
        function->setScope(popScope());
    }
}

Error *resolveVariableType(TypeTable* typeTable, VariableType *variableType) {
    if (variableType->getType() == STRUCT_TYPE) {
        VariableType *realType = typeTable->queryType(variableType->getTypeName());
        if (realType == nullptr) {
            return new Error{variableType->getLocation(),
                             "can not found complete definition for:" + variableType->getTypeName()};
        } else {
            variableType->setActualType(realType);
        }
    }
    return nullptr;
}


void TypeResolver::resolve(AST &ast) {
    for (DefinedVariable *var: ast.getDefinedVars()) {
        Error *_error = resolveVariableType(typeTable, var->getType());
        this->error(_error);
    }
    for(VariableType* declared: ast.getDeclaredTypes()){
        if(declared->getType()!=STRUCT_TYPE)
            continue;
        Struct* aStruct = (Struct*)declared;
        error(aStruct->checkMembers());
        for(DefinedVariable* member: aStruct->getMemberList()){
            Error *_error = resolveVariableType(typeTable, member->getType());
            this->error(_error);
        }
    }
    resolveFunctions(ast.defineFunctions());
}

void TypeResolver::resolveFunctions(list<DefinedFunction *> funs) {
    for (DefinedFunction *fun: funs) {
        for (DefinedVariable *para: fun->getParameters()) {
            Error *err = resolveVariableType(typeTable, para->getType());
            error(err);
        }
        Error *err = resolveVariableType(typeTable, fun->getReturnType());
        error(err);
        resolve(*fun->getBody());
    }
}

void TypeResolver::resolve(Body &body) {
    for (DefinedVariable *var:body.vars) {
        Error *err = resolveVariableType(typeTable, var->getType());
        error(err);
    }
    for (Statement *statement:body.statements) {
        resolveStatement(statement);
    }

}

TypeResolver::TypeResolver(ErrorHandler& errorHandle, TypeTable* type_table): Visitor(errorHandle, type_table) {

}

void TypeResolver::resolveStatement(Statement *statement) {
    if (statement->flag == BODY) {
        resolve(*(Body *) statement);
    }
}


