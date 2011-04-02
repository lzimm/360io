/*
 *  manager.h
 *  360sync
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _MANAGER_H_INCLUDED_
#define _MANAGER_H_INCLUDED_

#include "syncstore.h"

class Manager {
public:
    static void init();
    
public:
    static SyncStore store;
    
};

#endif
