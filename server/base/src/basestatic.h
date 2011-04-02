/*
 *  basestatic.h
 *  360base
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _BASESTATIC_H_INCLUDED_
#define _BASESTATIC_H_INCLUDED_

#include "shared/static.h"

class BaseStatic : public Static {
public:
    static void init();
    
public:
    static char*        s_landingTemplate;
    static char*        s_streamTemplate;
    
    static int          s_headerLen;
    static int          s_adjustedStreamTemplateLength;
    static int          s_landingTemplateLength;

};

#endif
