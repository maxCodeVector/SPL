#include "deliver.h"
#include <stdlib.h>
#include <malloc.h>


void show_tree_helper(struct AttrNode* node, int indent){ 
    if(node==NULL){
        return;
    }
    for(int i=0;i<indent;i++){
        printf("  ");
    }
    if(node->value==NULL){
        if (node->isTerminal!=1){
            printf("%s (%d)\n", node->name, node->lineNo);
        }else
        {
            printf("%s\n", node->name);
        }
        
    }else
    {
       printf("%s: %s\n", node->name, node->value);
    }
    
    struct AttrNode* child = node->firstChild;
    while(child!=NULL){
        show_tree_helper(child, indent+1);
        child = child->nextSibling;
    }
}

void show_sytax_tree(struct AttrNode* root){
    show_tree_helper(root, 0);
}

struct AttrNode* make_node(char* name){
    struct AttrNode* node = malloc(sizeof(struct AttrNode));
    node->name = strdup(name);
    return node;
}


struct AttrNode* make_parent(struct AttrNode* child, char* name){
    struct AttrNode* parent = malloc(sizeof(struct AttrNode));
    parent->name = strdup(name);
    parent->firstChild = child;
    parent->lineNo = child->lineNo;
    return parent;
}

void add_childs(struct AttrNode* parent, struct AttrNode* other_child){
    /* means it is a empty sentence */
    if(other_child->lineNo==-1){
        return;
    }
    struct AttrNode* child = parent->firstChild;
    while(child->nextSibling!=NULL){
        child = child->nextSibling;
    }
    child->nextSibling = other_child;
}