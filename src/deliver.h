#ifndef _DELIVER
#define _DELIVER
#include <stdlib.h>
#include <string>
using namespace std;

struct AttrNode{
    int isTerminal;
    float fv;
    char ch;
    string name;
    char* value;
    int lineNo;
    int child_num;
    struct AttrNode* firstChild;
    struct AttrNode* nextSibling;
};


struct AttrNode* make_node(string name);
struct AttrNode* make_parent(struct AttrNode* child, string name);
void add_childs(struct AttrNode* parent, struct AttrNode* other_child);

void show_sytax_tree(struct AttrNode* root);

#endif