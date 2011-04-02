/*
 *  logstatic.cpp
 *  360log
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "logstatic.h"
#include "shared/util.h"

#include <string.h>
#include <stdio.h>

char* LogStatic::s_bridgeTemplate;
char* LogStatic::s_childTemplate;

void LogStatic::init() {
    char* bridgeBuffer              = read("/Life/360io/server/comm/static/bridge.tmpl");
    int bridgeLength                = Util::adjustedLen(bridgeBuffer, Static::s_headerLen);
    int fullBLen                    = strlen(bridgeBuffer) +  Util::intWidth(bridgeLength);
    
    s_bridgeTemplate                = new char[fullBLen + 1];
    *(s_bridgeTemplate + fullBLen)  = '\0';
    
    sprintf(s_bridgeTemplate, bridgeBuffer, bridgeLength);  
    delete[] bridgeBuffer;
    
    char* childBuffer               = read("/Life/360io/server/comm/static/child.tmpl");
    int childLength                 = Util::adjustedLen(childBuffer, Static::s_headerLen);
    int fullCLen                    = strlen(childBuffer) + Util::intWidth(childLength);
    
    s_childTemplate                 = new char[fullCLen + 1];
    *(s_childTemplate + fullCLen)   = '\0';
    
    sprintf(s_childTemplate, childBuffer, childLength); 
    delete[] childBuffer;
}
