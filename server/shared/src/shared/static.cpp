/*
 *  static.cpp
 *  360io
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "static.h"
#include "util.h"

#include <err.h>
#include <fstream>

int Static::s_headerLen = strlen(HTTP_HEADERS);

char* Static::read(char* path) {
    std::ifstream file(path, std::ios::in|std::ios::binary|std::ios::ate);
    
    if (!file.is_open()) err(1, path);
    
    int size            = file.tellg();
    int bufferSize      = size + s_headerLen + Util::intWidth(size);
    char* buffer        = new char[bufferSize];
    char* bufferStart   = buffer + s_headerLen;
    
    memset(buffer, 0, bufferSize);
    memcpy(buffer, HTTP_HEADERS, s_headerLen);
    
    file.seekg(0, std::ios::beg);
    file.read(bufferStart, size);
    file.close();
    
    return buffer;
}
