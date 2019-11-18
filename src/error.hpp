#ifndef __SPLERROR__
#define __SPLERROR__

#include <string>
#include <iostream>
#include "includes.h"

struct Error{
    Location* loc;
    string message;
};

class ErrorHandler{
private:
    int errorNum = 0;
    list<Error*> errorList;

public:
    bool errorOccured(){
        return errorNum > 0;
    }

    void recordError(Location* loc, string& message){
        Error* error = new Error{loc, message};
        errorList.push_back(error);
    }

    void showError(){
        for(Error* error: errorList){
            std::cerr << "error in:" << error->loc->toString() << ", " << error->message << endl;
        }
    }

    ~ErrorHandler(){
        free_all(errorList);
    }


};





#endif