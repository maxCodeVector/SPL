#ifndef __SPLERROR__
#define __SPLERROR__

class ErrorHandler{
private:
    int errorNum = 0;

public:
    bool errorOccured(){
        return errorNum > 0;
    }
};



#endif