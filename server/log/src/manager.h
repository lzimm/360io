/*
 *  manager.h
 *  360log
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _MANAGER_H_INCLUDED_
#define _MANAGER_H_INCLUDED_

#include "logstore.h"

class Manager {
public:
    static void init();
    
public:
    static LogStore store;
    
};

#endif
