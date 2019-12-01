//
// Created by hya on 11/22/19.
//

#include "resolver.h"


void addBuildFunctions(AST &ast) {
    list<Function *> &functions = ast.defineFunctions();
    Function *read = getBuildFunction("read", new VariableType(INT_TYPE));
    Function *write = getBuildFunction("write", new VariableType(INT_TYPE));

    Variable *var = new Variable("n", INT_TYPE);
    write->parameters.push_back(var);

    functions.push_back(read);
    functions.push_back(write);
}

void LocalResolver::resolve(AST &ast) {
    ToplevelScope *toplevelScope = new ToplevelScope();
    scopeStack.push_back(toplevelScope);

    addBuildFunctions(ast);
    list<Entity *> entities;
    for (Entity *e: ast.declaritions(entities)) {
        Entity *hasE = toplevelScope->declareEntity(*e);
        if (hasE != nullptr) {
            string message =
                    "duplicated define for:" + e->getName() + ", last defined in:" + hasE->getLocation()->toString();
            ErrorType errorType = typeid(Function) == typeid(*e) ? REDEFINED_FUN : REDEFINED_VAR;
            error(e->getLocation(), errorType, message);
        }
    }
    resolveDeclaredType(ast.getDeclaredTypes());
    resolveFunctions(ast.defineFunctions());
    ast.setScope(toplevelScope);
    ast.setConstant(this->constantTable);
}


void LocalResolver::resolveDeclaredType(list<VariableType *> &declared) {
    for (VariableType *variableType: declared) {
        if (variableType->getElementType() != STRUCT_TYPE)
            continue;
        // unique member
        typeTable->declareVariableType(variableType, errorHandler);
    }
}


void LocalResolver::resolveFunctions(list<Function *> &funcs) {
    for (Function *function: funcs) {
        pushScope(function->getParameters());
        if (function->flag != BUILD_NODE)
            resolve(*function->getBody());
        function->setScope(popScope());
    }
}

void LocalResolver::resolve(Body &body) {
    LocalScope *curr = (LocalScope *) currentScope();
    for (Variable *var: body.vars) {
        if (curr->isDefinedLocally(var->getName())) {
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), REDEFINED_VAR, message);
        } else {
            curr->defineVariable(*var);
        }
    }
    for (Variable *var: body.vars) {
        if (var->getValue()) {
            // check defined variables' initial expression
            error(var->getValue()->checkReference(currentScope()));
        }
    }
    for (Statement *statement:body.statements) {
        statement->checkReference(this, currentScope());
    }
}

void LocalResolver::pushScope(list<Variable *> &vars) {
    Scope *parent = currentScope();
    LocalScope *scope = new LocalScope(parent);
    parent->children.emplace_back(scope);

    for (Variable *var: vars) {
        if (scope->isDefinedLocally(var->getName())) {
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), REDEFINED_VAR, message);
        } else {
            scope->defineVariable(*var);
        }
    }
    scopeStack.push_back(scope);
}

Scope *LocalResolver::popScope() {
    Scope *scope = scopeStack.back();
    scopeStack.pop_back();
    return scope;
}


TypeResolver::TypeResolver(ErrorHandler &errorHandle, TypeTable *type_table) : Visitor(errorHandle, type_table) {
}

Error *resolveVariableType(TypeTable *typeTable, VariableType *variableType) {
    if (variableType->getElementType() == STRUCT_TYPE) {
        VariableType *realType = typeTable->queryType(variableType->getTypeName());
        if (realType == nullptr) {
            return new Error{variableType->getLocation(), ErrorType::INCOMPLETE_STRUCT,
                             "can not found complete definition for:" + variableType->getTypeName()};
        } else {
            ((Struct *) variableType)->setActualType(realType);
        }
    } else if (variableType->getElementType() == ARRAY_TYPE) {
        return resolveVariableType(typeTable, variableType->getElement());
    }
    return nullptr;
}

void TypeResolver::resolve(AST &ast) {
    // non-struct elementType already has its actual elementType, so only deal with struct elementType
    // by query it from elementType table, set referenced elementType's actual elementType
    for (Variable *var: ast.getDefinedVars()) {
        Error *_error = resolveVariableType(typeTable, var->getType());
        this->error(_error);
    }
    for (VariableType *declared: ast.getDeclaredTypes()) {
        if (declared->getElementType() != STRUCT_TYPE)
            continue;
        Struct *aStruct = (Struct *) declared;
        // check if struct has duplicated name members
        error(aStruct->checkDuplicatedNameMember());
        for (Variable *member: aStruct->getMemberList()) {
            // set each member's referenced elementType to its actual elementType
            Error *_error = resolveVariableType(typeTable, member->getType());
            this->error(_error);
        }
    }
    resolveFunctions(ast.defineFunctions());
}

void TypeResolver::resolveFunctions(list<Function *> &funs) {
    for (Function *fun: funs) {
        for (Variable *para: fun->getParameters()) {
            Error *err = resolveVariableType(typeTable, para->getType());
            error(err);
        }
        Error *err = resolveVariableType(typeTable, fun->getReturnType());
        error(err);
        if (fun->flag != BUILD_NODE)
            resolve(*fun->getBody());
    }
}

void TypeResolver::resolve(Body &body) {
    for (Variable *var:body.vars) {
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


