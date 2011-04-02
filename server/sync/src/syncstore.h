/*
 *  syncstore.h
 *  360sync
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _SYNCSTORE_H_INCLUDED_
#define _SYNCSTORE_H_INCLUDED_

#include "shared/store.h"

class SyncStore : public Store {
public:
    SyncStore();
    ~SyncStore();
    
public:
    DEFINE_DB_INTERFACE(Count, m_countDB)
    
private:
    Db*         m_countDB;
    
};

#endif
