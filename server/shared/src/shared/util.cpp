/*
 *  util.cpp
 *  360io
 *
 *  Created by Lewis Zimmerman on 09/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "util.h"
#include "string.h"

#include <iostream>

#include <sys/time.h>
#include <sys/resource.h>

int Util::intWidth(int num) {
    int lens[7] = {0, 9, 99, 999, 9999, 99999, 999999};
    int len = 3;
    int adjust = 1;
    
    if (num > lens[len]) {
        adjust = 0;
        while (num > lens[++len]);
    } else while (num <= lens[--len]);
    
    return len + adjust;
}

int Util::adjustedLen(char* body, int baseLen) {
    int adjustedLen     = 0;
    int len             = strlen(body);
    
    for (char* p = body + baseLen; p < body + len; ++p) {
        ++adjustedLen;
        if (*p == '%') ++p;
    }
    
    return adjustedLen;
}

double Util::getTime() {
    struct timeval utim;
    gettimeofday(&utim, NULL);
    return (double) utim.tv_sec * 1000000.0 + (double) utim.tv_usec;
}

void Util::logTime(char* family, char* label, double start) {
    char* buff = new char[strlen(family) + strlen(label) + 30];
    sprintf(buff, "%s : %s -- %fus\n", family, label, getTime() - start);
    std::clog << buff;
    delete[] buff;
}
