/*
 *  logstore.cpp
 *  360log
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "logstore.h"

#include <sstream>
#include <set>

#define DB_LOGIDXTYPE   DB_BTREE
#define DB_LOGIDXFLAG   DB_DUP

#define DB_LOG          "logdb.db"

#define DATA_NULL       0x6C6C756E

void* LogStore::pull(char* idx, int len) {
    std::stringstream stream;

    Dbc *cursor;
    m_logDB->cursor(NULL, &cursor, 0);
    
    Dbt key(idx, strlen(idx));
    Dbt data;
    
    stream << "[";
    
    int count           = 0;
    int ret             = cursor->get(&key, &data, DB_SET);

    while (ret != DB_NOTFOUND && count < len) {
        char* logData   = static_cast<char*>(data.get_data());        
        
        stream << (count++ > 0 ? "," : "") << logData;
        
        ret             = cursor->get(&key, &data, DB_NEXT_DUP);
    }
    
    if (cursor != NULL) cursor->close();
    
    stream << "]";
    
    char* buff          = new char[stream.str().size() + 1];
    strcpy(buff, stream.str().c_str());

    return buff;
}

bool LogStore::push(int len, char* key, void* data) {
    Dbt _key(key, strlen(key));
    Dbt _data(data, len);

    DbTxn *txn; 
    Dbc *cursor;
    
    m_env.txn_begin(NULL, &txn, 0);
    m_logDB->cursor(txn, &cursor, 0);
    
    int ret = cursor->put(&_key, &_data, DB_KEYFIRST);

    if (cursor != NULL) cursor->close();
    if (txn != NULL) txn->commit(0);
    
    return ret == 0;
}

LogStore::LogStore() : Store::Store() {
    try {
        initDB(DB_LOG, m_logDB, DB_LOGIDXTYPE, DB_LOGIDXFLAG);
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

LogStore::~LogStore() {
    m_logDB->close(0);
    delete m_logDB;
}
