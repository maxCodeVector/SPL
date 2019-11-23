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
            ErrorType errorType = e->flag==FUNC? REDEFINED_FUN:REDEFINED_VAR;
            error(e->getLocation(),errorType, message);
        }
    }
    resolveDeclaredType(ast.getDeclaredTypes());
    resolveFunctions(ast.defineFunctions());
    ast.setScope(toplevelScope);
    ast.setConstant(this->constantTable);
}


void LocalResolver::resolveDeclaredType(list<VariableType *> &declared) {
    for (VariableType *variableType: declared) {
        if(variableType->getType()!=STRUCT_TYPE)
            continue;
        // unique member
        typeTable->declareVariableType(variableType, errorHandler);
    }
}


void LocalResolver::resolveFunctions(list<DefinedFunction *> &funcs) {
    for (DefinedFunction *function: funcs) {
        pushScope(function->getParameters());
        resolve(*function->getBody());
        function->setScope(popScope());
    }
}

void LocalResolver::resolve(Body &body) {
    LocalScope *curr = (LocalScope *) currentScope();
    for (DefinedVariable *var: body.vars) {
        if (curr->isDefinedLocally(var->getName())) {
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), REDEFINED_VAR, message);
        } else {
            curr->defineVariable(*var);
        }
    }
    for (DefinedVariable *var: body.vars) {
        if (var->getValue()) {
            // check defined variables' initial expression
            error(var->getValue()->checkReference(currentScope()));
        }
    }
    for (Statement *statement:body.statements) {
        statement->checkReference(this, currentScope());
    }
}

void LocalResolver::pushScope(list<DefinedVariable *> &vars) {
    Scope *parent = currentScope();
    LocalScope *scope = new LocalScope(parent);
    parent->children.emplace_back(scope);

    for (DefinedVariable *var: vars) {
        if (scope->isDefinedLocally(var->getName())) {
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), REDEFINED_VAR,  message);
        } else {
            scope->defineVariable(*var);
        }
    }
    scopeStack.push_back(scope);
}

Scope *LocalResolver::popScope() {
    Scope* scope = scopeStack.back();
    scopeStack.pop_back();
    return scope;
}


TypeResolver::TypeResolver(ErrorHandler& errorHandle, TypeTable* type_table): Visitor(errorHandle, type_table) {
}

Error *resolveVariableType(TypeTable* typeTable, VariableType *variableType) {
    if (variableType->getType() == STRUCT_TYPE) {
        VariableType *realType = typeTable->queryType(variableType->getTypeName());
        if (realType == nullptr) {
            return new Error{variableType->getLocation(), ErrorType ::INCOMPLETE_STRUCT,
                             "can not found complete definition for:" + variableType->getTypeName()};
        } else {
            variableType->setActualType(realType);
        }
    }
    return nullptr;
}

void TypeResolver::resolve(AST &ast) {
    // non-struct type already has its actual type, so only deal with struct type
    // by query it from type table, set referenced type's actual type
    for (DefinedVariable *var: ast.getDefinedVars()) {
        Error *_error = resolveVariableType(typeTable, var->getType());
        this->error(_error);
    }
    for(VariableType* declared: ast.getDeclaredTypes()){
        if(declared->getType()!=STRUCT_TYPE)
            continue;
        Struct* aStruct = (Struct*)declared;
        // check if struct has duplicated name members
        error(aStruct->checkDuplicatedNameMember());
        for(DefinedVariable* member: aStruct->getMemberList()){
            // set each member's referenced type to its actual type
            Error *_error = resolveVariableType(typeTable, member->getType());
            this->error(_error);
        }
    }
    resolveFunctions(ast.defineFunctions());
}

void TypeResolver::resolveFunctions(list<DefinedFunction *>& funs) {
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

void TypeResolver::resolveStatement(Statement *statement) {
    // only need to check body statement, because other statement do not have variable declarations.
    if (statement->flag == BODY) {
        resolve(*(Body *) statement);
    }
}


