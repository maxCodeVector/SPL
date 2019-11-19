//
// Created by hya on 11/19/19.
//
#include "parse/deliver.h"
#include "semantic.h"


int main(int argc, char **argv){
    AttrNode* parse_tree_root = get_parse_tree(argc, argv);
    if(parse_tree_root== nullptr){
        return 1;
    }
    show_syntax_tree(parse_tree_root);
    semantic_analysis(parse_tree_root);
    free_AttrNode(parse_tree_root);
}

