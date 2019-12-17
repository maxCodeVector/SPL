//
// Created by hya on 12/11/19.
//

#ifndef SPL_BUILD_H
#define SPL_BUILD_H

#include <stdio.h>
int write(int n){
    printf("%d\n", n);
}

int read(){
    int res;
    scanf("%d", &res);
    return res;
}

#endif //SPL_BUILD_H
