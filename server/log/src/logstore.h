/*
 *  logstore.h
 *  360log
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _LOGSTORE_H_INCLUDED_
#define _LOGSTORE_H_INCLUDED_

#include "shared/store.h"
#include "shared/util.h"

class LogStore : public Store {
public:
    LogStore();
    ~LogStore();
    
public:
    void* pull(char*, int);
    bool push(int, char*, void*);

private:
    Db*         m_logDB;
    
};

#endif
