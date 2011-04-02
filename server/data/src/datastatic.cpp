/*
 *  datastatic.cpp
 *  360data
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "datastatic.h"
#include "shared/util.h"

#include <string.h>
#include <stdio.h>

char* DataStatic::s_bridgeTemplate;
char* DataStatic::s_childTemplate;
char* DataStatic::s_crossDomainXML;

void DataStatic::init() {
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
    
    char* crossdomainBuffer         = read("/Life/360io/server/comm/static/crossdomain.xml");
    int crossdomainBufferLength     = Util::adjustedLen(crossdomainBuffer, Static::s_headerLen);
    int fullXLen                    = strlen(crossdomainBuffer) + Util::intWidth(crossdomainBufferLength);
    
    s_crossDomainXML                = new char[fullXLen + 1];
    *(s_crossDomainXML + fullXLen)  = '\0';
    
    sprintf(s_crossDomainXML, crossdomainBuffer, crossdomainBufferLength); 
    delete[] crossdomainBuffer;
}
