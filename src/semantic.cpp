#include "semantic.h"
#include "error.h"
#include "type.h"
#include "resolver.h"

using namespace std;


int semantic_analysis(AST &ast) {
    ErrorHandler h = ErrorHandler();

    LocalResolver local(h);
    TypeResolver typeResolver(h);
    DereferenceChecker dereferenceChecker(h);
    TypeChecker typeChecker(h);

    local.resolve(ast);
    typeResolver.resolve(ast);
    //check recursive definition
    ast.getScope()->checkReferences(h);
    dereferenceChecker.resolve(ast);
    typeChecker.resolve(ast);

    h.showError(std::cout);
//    ofstream outfile("../src/res.o");
//    h.showError(outfile);
    return 1;
}