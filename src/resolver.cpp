//
// Created by hya on 11/22/19.
//

#include "resolver.h"

void LocalResolver::resolve(AST &ast) {
    ToplevelScope* toplevelScope = new ToplevelScope();
    scopeStack.push_back(toplevelScope);

    list<Entity*> entities;
    for(Entity* e: ast.declaritions(entities)){
        Entity* hasE = toplevelScope->declareEntity(*e);
        if(hasE!= nullptr){
            string message = "duplicated define for:"+e->getName()+", last defined in:"+ hasE->getLocation()->toString();
            error(e->getLocation(), message);
        }
    }

    resolveGloableVarIntializers();
    resolveConstantValues();
    resolveDeclaredType(toplevelScope, ast.getDeclaredTypes());
    resolveFunctions(ast.defineFunctions());
    toplevelScope->checkReferences(this->errorHandler);
//        if(errorHandler.errorOccured()){
//            return;
//        }
    ast.setScope(toplevelScope);
    ast.setConstant(this->constantTable);
}

void LocalResolver::resolve(Body &body){
    LocalScope* curr = (LocalScope*)currentScope();
    for(DefinedVariable* var: body.vars){
        if(curr->isDefinedLocally(var->getName())){
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), message);
        }else{
            curr->defineVariable(*var);
        }
    }
    for(DefinedVariable* var: body.vars){
        if(var->getValue()){
            error(var->getValue()->checkReference(currentScope()));
        }
    }
    for (Statement* statement:body.statements){
        resolve(*statement);
    }
}

void LocalResolver::pushScope(list<DefinedVariable *> &vars){
    Scope* parent = currentScope();
    LocalScope* scope = new LocalScope(parent);
    parent->children.emplace_back(scope);

    for(DefinedVariable* var: vars){
        if(scope->isDefinedLocally(var->getName())){
            string message = "duplicated variable in scope：" + var->getName();
            error(var->getLocation(), message);
        }else{
            scope->defineVariable(*var);
        }
    }
    scopeStack.push_back(scope);
}

void LocalResolver::resolveDeclaredType(ToplevelScope *toplevel, list<VariableType *> &declared){
    for(VariableType* variableType: declared){
        toplevel->declareVariableType(variableType, errorHandler);
    }
}

void LocalResolver::resolve(Statement &statement) {
    Exp* exp = statement.getExpression();
    Error* _error = exp->checkReference(currentScope());
    if(_error){
        error(_error);
    }
}

void LocalResolver::resolveFunctions(list<DefinedFunction *> &funcs){
    for(DefinedFunction* function: funcs){
        pushScope(function->getParameters());
        resolve(*function->getBody());
        function->setScope(popScope());
    }
}


void TypeResolver::resolve(AST &ast) {
    ToplevelScope* toplevelScope = ast.getScope();
    for(DefinedVariable* var: ast.getDefinedVars()){
        VariableType* variableType = var->getType();
        if(variableType->getType()==STRUCT_TYPE){
            VariableType* realType = toplevelScope->queryType(variableType->getTypeName());
            if(realType== nullptr){
                this->error(new Error{variableType->getLocation(),
                                      "can not found complete definition for:"+variableType->getTypeName()});
            } else{
                var->setActualType(realType);
            }
        }
    }

}

void TypeResolver::resolve(Body &body) {

}

TypeResolver::TypeResolver(ErrorHandler &errorHandle) : Visitor(errorHandle) {

}