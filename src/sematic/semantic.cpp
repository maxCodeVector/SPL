#include "semantic.h"
#include "../error.h"
#include "type.h"
#include "resolver.h"

using namespace std;


ErrorHandler* semantic_analysis(AST &ast) {
    ErrorHandler* h = new ErrorHandler();
    TypeTable *typeTable = new TypeTable;

    LocalResolver local(*h, typeTable);
    TypeResolver typeResolver(*h, typeTable);
    DereferenceChecker dereferenceChecker(*h, typeTable);
    TypeChecker typeChecker(*h, typeTable);

    local.resolve(ast);
    typeResolver.resolve(ast);
    //check recursive definition
    typeTable->checkRecursiveDefinition(*h);
    if (!h->errorOccured()) {
//        dereferenceChecker.resolve(ast);
        typeChecker.resolve(ast);
    }
    ast.setTypeTable(typeTable);
    return h;
}