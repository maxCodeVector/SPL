#ifndef __SPLERROR__
#define __SPLERROR__

#include <string>
#include <iostream>
#include "includes.h"


class ErrorHandler{
private:
    list<Error*> errorList;

public:
    bool errorOccured(){
        return !errorList.empty();
    }

    void recordError(Location* loc, ErrorType  type, string& message){
        Error* error = new Error{loc, type, message};
        errorList.push_back(error);
    }

    void recordError(Error* error){
        errorList.push_back(error);
    }

    void showError(ostream &basicOstream) {
        for(Error* error: errorList){
            basicOstream << "Error type " << error->errorType <<" at Line " << error->loc->toString() << ": " << error->message << endl;
        }
    }

    ~ErrorHandler(){
        free_all(errorList);
    }


};





#endif