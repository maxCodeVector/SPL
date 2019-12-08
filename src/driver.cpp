//
// Created by hya on 11/19/19.
//
#include "parse/deliver.h"
#include "semantic/semantic.h"
#include "ir/IRGenerator.h"
#include <fstream>

int main(int argc, char **argv){
    AttrNode* parse_tree_root = get_parse_tree(argc, argv);
    if(parse_tree_root== nullptr){
        return 1;
    }

//    show_syntax_tree(parse_tree_root);
    AST* ast = (AST*)parse_tree_root->baseNode;
    ErrorHandler* handler = semantic_analysis(*ast);

    string out_file_name = argv[1];
    out_file_name = out_file_name.substr(0, out_file_name.size()-3)+"out";
    ofstream outfile(out_file_name);
    if(handler->errorOccured()) {
        //    handler->showError(outfile);
        handler->showError(std::cerr);
        return 1;
    }

    IRGenerator irGenerator;
    IR* ir = irGenerator.generate(*ast);
    ir->write(std::cout);
//    ir->write(outfile);
    delete(ast);
    delete(handler);
    free_AttrNode(parse_tree_root);
}

