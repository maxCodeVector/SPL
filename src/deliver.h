#ifndef _DELIVER
#define _DELIVER
#include <stdlib.h>

struct AttrNode{
    int isTerminal;
    float fv;
    char ch;
    char* name;
    char* value;
    int lineNo;
    int child_num;
    struct AttrNode* firstChild;
    struct AttrNode* nextSibling;
};


struct AttrNode* make_node(char* name);
struct AttrNode* make_parent(struct AttrNode* child, char* name);
void add_childs(struct AttrNode* parent, struct AttrNode* other_child);

void show_sytax_tree(struct AttrNode* root);

#endif