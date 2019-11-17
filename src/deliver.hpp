#ifndef _DELIVER
#define _DELIVER
#include <stdlib.h>
#include <string>
using namespace std;

class BaseNode;

struct AttrNode{
    int isTerminal;
    float fv;
    char ch;
    string name;
    char* value;
    string childsName;
    int lineNo;
    int child_num;
    struct AttrNode* firstChild;
    struct AttrNode* nextSibling;
    BaseNode* baseNode;
};


struct AttrNode* make_node(string name);
struct AttrNode* make_parent(struct AttrNode* child, string name);
struct AttrNode* get_child(struct AttrNode* parent, int index);
void add_childs(struct AttrNode* parent, struct AttrNode* other_child);


void show_sytax_tree(struct AttrNode* root);

#endif