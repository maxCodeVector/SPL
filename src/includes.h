//
// Created by hya on 11/18/19.
//

#ifndef SPL_INCLUDES_H
#define SPL_INCLUDES_H
#include <list>
#include <string>
#include "parse/deliver.h"
using namespace std;

class Location {
private:
    int lineNo;
public:
    Location(int line, int col) {
        this->lineNo = line;
    }

    string toString() {
        return "line " + to_string(lineNo);
    }
};

struct Error{
    Location* loc;
    string message;
};


class ConstantEntry {
    string name;
    string value;
};

class ConstantTable {
    list<ConstantEntry> entries;
};

enum NodeType {
    OTHER,
    VAR,
    FUNC,
    DECLARATION,
    BODY
};

enum DataType{
    DER_TYPE, // means it is derivative type
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    STRUCT_TYPE,
    OTHER_TYPE // only ID expression has this type
};


enum Operator{
    UNKNOWN_OP,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    NOT_OP,
    INVOKE,
    ASSIGN_OP,
    AND_OP,
    OR_OP,
    LT_OP,
    LE_OP,
    GT_OP,
    GE_OP,
    NE_OP,
    EQ_OP,
    CONT_OP,
    BREAK_OP,
    DOT_OP,
    ARRAY_INDEX_OP
};

template <typename T>
void free_all(list<T*>& element_list){
    while (!element_list.empty()){
        T* element = element_list.back();
        element_list.pop_back();
        delete(element);
    }
}

#endif //SPL_INCLUDES_H
