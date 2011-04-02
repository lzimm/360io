/*
 *  basestatic.cpp
 *  360base
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "basestatic.h"
#include "shared/util.h"

#include <err.h>
#include <fstream>

char* BaseStatic::s_landingTemplate;
char* BaseStatic::s_streamTemplate;

int BaseStatic::s_adjustedStreamTemplateLength = 0;
int BaseStatic::s_landingTemplateLength = 0;

void BaseStatic::init() {
    char* buffer                    = read("/Life/360io/server/base/static/landing.tmpl");
    int contentLength               = Util::adjustedLen(buffer, Static::s_headerLen);
    int fullLen                     = strlen(buffer) + Util::intWidth(contentLength);
    
    s_landingTemplate               = new char[fullLen + 1];
    *(s_landingTemplate + fullLen)  = '\0';
    
    sprintf(s_landingTemplate, buffer, contentLength);
    s_landingTemplateLength = strlen(s_landingTemplate);
    
    s_streamTemplate                = read("/Life/360io/server/base/static/stream.tmpl");
    s_adjustedStreamTemplateLength  = Util::adjustedLen(s_streamTemplate, Static::s_headerLen);
    
    delete[] buffer;
}
