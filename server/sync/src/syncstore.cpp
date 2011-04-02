/*
 *  syncstore.cpp
 *  360sync
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "syncstore.h"

#define DB_INDEXTYPE    DB_HASH
#define DB_INDEXFLAG    0

#define DB_COUNT        "countdb.db"

SyncStore::SyncStore() : Store::Store() {
    try {
        initDB(DB_COUNT, m_countDB, DB_INDEXTYPE, DB_INDEXFLAG);
    } catch (DbException &e) {
        std::cerr   << "Error opening database environment: "
        << DB_HOME << std::endl;
        std::cerr   << e.what() << std::endl;
        exit(-1);
    } catch (std::exception &e) {
        std::cerr   << "Error opening database environment: "
        << DB_HOME << std::endl;
        std::cerr   << e.what() << std::endl;
        exit(-1);
    }
}

SyncStore::~SyncStore() {
    m_countDB->close(0);
    
    delete m_countDB;
}
