/*
 *  datastatic.h
 *  360data
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _DATASTATIC_H_INCLUDED_
#define _DATASTATIC_H_INCLUDED_

#include "shared/static.h"

class DataStatic : public Static {
public:
    static void init();
    
public:
    static char*        s_bridgeTemplate;
    static char*        s_childTemplate;
    static char*        s_crossDomainXML;
    
};

#endif
