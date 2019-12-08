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
    Location(int line, int col) {
        this->lineNo = line;
    }
public:

    string toString() {
        return to_string(lineNo);
    }
    friend class BaseNode;
};

enum ErrorType{
    UNDEFINED_VAR=1,
    UNDEFINED_FUN,
    REDEFINED_VAR,
    REDEFINED_FUN,
    ASSIGN_DIFF_TYPE,
    ASSIGN_LEFT_VALUE,
    UNMATCHED_OPERATE,
    MIS_RETURN_TYPE,
    MIS_ARGUMENT,
    NON_ARRAY,
    NON_FUNC,
    NON_INT_INDEX,
    NON_STRUCT,
    UNDEFINED_MEMBER,
    REDEFINED_STRUCT,
    INCOMPLETE_STRUCT,
    RECURSIVE_DEFINE,
    OTHER_ERROR
};

enum ErrorCode{
    NULL_LOCATION=21,
    ARRAY_TYPE_ARRAY
};


struct Error{
    Location* loc;
    ErrorType errorType;
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
    BODY,
    BUILD_NODE
};

enum DataType{
    INFER_TYPE, // means it is derivative elementType, need to be inferred from other expressions
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    STRUCT_TYPE,
    REF_TYPE, //  ID expression has this elementType or other expression that can inferred to this elementType,
    // if inferred, elementType is REF_TYPE, before inferred, it is INFER_TYPE.
    BOOL_TYPE,
    ARRAY_TYPE
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


template <typename T>
void copy_all(list<T*>& element_list, list<T*>& dest){
    while (!element_list.empty()){
        T* element = element_list.front();
        element_list.pop_front();
        dest.push_back(element);
    }
}
#endif //SPL_INCLUDES_H
