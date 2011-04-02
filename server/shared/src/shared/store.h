/*
 *  store.h
 *  360io
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _STORE_H_INCLUDED_
#define _STORE_H_INCLUDED_

#include <stdint.h>

#include <db_cxx.h>

#define DB_HOME         "/Life/360io/db"
#define ENV_FLAGS       DB_CREATE | DB_INIT_MPOOL | DB_INIT_LOCK | DB_INIT_TXN
#define DB_FLAGS        DB_CREATE | DB_AUTO_COMMIT | DB_READ_UNCOMMITTED

#define DEFINE_DB_INTERFACE(db_name, db) \
    inline bool add##db_name(int len, char* key, void* data) { \
        return add(db, len, key, data); \
    } \
    inline void put##db_name(int len, char* key, void* data) { \
        put(db, len, key, data); \
    } \
    inline void* get##db_name(char* key) { \
        void* data; \
        get(db, key, data); \
        return data; \
    } \
    inline void append##db_name(int len, char* key, void* data) { \
        void* old; \
        int oldLen = get(db, key, old); \
        char* buff = static_cast<char*>(malloc(oldLen + len)); \
        memcpy(buff, old, oldLen); \
        memcpy(buff + oldLen, data, len); \
        put(db, oldLen + len, key, buff); \
        free(old); \
        free(buff); \
    }

class Store {
public:
    Store();
    ~Store();
    
public:
    bool add(Db*, int, char*, void*);
    void put(Db*, int, char*, void*);
    int get(Db*, char*, void*&);
    
protected:
    void initDB(char*, Db*&, DBTYPE, uint32_t);
    
protected:
    DbEnv       m_env;
    
};

#endif
