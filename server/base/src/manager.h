/*
 *  manager.h
 *  360base
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _MANAGER_H_INCLUDED_
#define _MANAGER_H_INCLUDED_

#include "shared/conn.h"

class Manager {
public:
    static void init();
    
public:
    static Conn* acquireDataConnection();
    static Conn* acquireCommConnection();

private:
    static ConnPool     m_dataPool;
    static ConnPool     m_commPool;
    
};

#endif
