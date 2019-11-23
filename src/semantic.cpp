#include "semantic.h"
#include "error.h"
#include "type.h"
#include "resolver.h"

using namespace std;


int semantic_analysis(AST &ast) {
    ErrorHandler h = ErrorHandler();
    TypeTable *typeTable = new TypeTable;

    LocalResolver local(h, typeTable);
    TypeResolver typeResolver(h, typeTable);
    DereferenceChecker dereferenceChecker(h, typeTable);
    TypeChecker typeChecker(h, typeTable);

    local.resolve(ast);
    typeResolver.resolve(ast);
    //check recursive definition
    typeTable->checkRecursiveDefinition(h);
//    dereferenceChecker.resolve(ast);
    typeChecker.resolve(ast);

    h.showError(std::cerr);
    delete(typeTable);
//    ofstream outfile("../src/res.o");
//    h.showError(outfile);
    return 1;
}