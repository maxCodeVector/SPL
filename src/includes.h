//
// Created by hya on 11/18/19.
//

#ifndef SPL_INCLUDES_H
#define SPL_INCLUDES_H
#include <list>
#include <string>
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

class ConstantEntry {
    string name;
    string value;
};

class ConstantTable {
    list<ConstantEntry> entries;
};

enum NodeType {
    VAR,
    FUNC,
    OTHER
};

enum DataType{
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    STRUCT_TYPE,
    OTHER_TYPE
};


enum Operator{
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
    EQ_OP
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
