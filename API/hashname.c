//
//  hashname.c
//  dulang
//
//  Created by Дмитрий Маслюков on 13/06/2019.
//  Copyright © 2019 Дмитрий Маслюков. All rights reserved.
//

#include "../api.h"
#include <string.h>
unsigned int hashstr(const char*str){
    int res = 0;
    for(int i = 0; i<strlen(str);++i){
        res*=POL;
        res+=(str[i]-'A');
    }
    return res;
}

unsigned int hashname_n(const char * str, int len){
    int res = 0;
    for(int i = 0; i<len;++i){
        res*=POL;
        res+=(str[i]-'A');
    }
    return res;
}
