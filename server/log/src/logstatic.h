/*
 *  logstatic.h
 *  360log
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _LOGSTATIC_H_INCLUDED_
#define _LOGSTATIC_H_INCLUDED_

#include "shared/static.h"

class LogStatic : public Static {
public:
    static void init();
    
public:
    static char*        s_bridgeTemplate;
    static char*        s_childTemplate;
    
};

#endif
