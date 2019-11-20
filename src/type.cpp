//
// Created by hya on 11/19/19.
//

#include "type.h"
#include "astnode/statement.h"

void DereferenceChecker::resolve(AST &ast) {
    for(DefinedVariable* var: ast.getDefinedVars()){

    }
    for(DefinedFunction* function: ast.defineFunctions()){
        resolve(*function->getBody());
    }
}



void DereferenceChecker::resolve(Body &body) {
    for(Statement* statement:body.statements){
        statement->checkMembersType(errorHandler);
    }

}

DereferenceChecker::DereferenceChecker(ErrorHandler &handler): Visitor(handler) {

}
