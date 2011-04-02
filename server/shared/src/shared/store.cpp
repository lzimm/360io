/*
 *  store.cpp
 *  360io
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "store.h"

Store::Store() : m_env(0) {
    try {
        m_env.open(DB_HOME, ENV_FLAGS, 0);
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

Store::~Store() {
    m_env.close(0);
}

bool Store::add(Db* db, int len, char* key, void* data) {
    Dbt _key(key, strlen(key));
    Dbt _data(data, len);
    
    int ret = db->put(NULL, &_key, &_data, DB_NOOVERWRITE);
    
    if (ret == DB_KEYEXIST) {
        return false;
    } else {
        return true;
    }
}

void Store::put(Db* db, int len, char* key, void* data) {
    Dbt _key(key, strlen(key));
    Dbt _data(data, len);
    
    db->put(NULL, &_key, &_data, 0);
}

int Store::get(Db* db, char* key, void*& data) {
    Dbt _key(key, strlen(key));
    Dbt _data;
    
    _data.set_flags(DB_DBT_MALLOC);
    
    int ret = db->get(NULL, &_key, &_data, DB_READ_UNCOMMITTED);
    
    if (ret == DB_NOTFOUND) {
        data = NULL;
        return -1;
    }
    
    data = _data.get_data();
    return _data.get_ulen();
}

void Store::initDB(char* dbName, Db*& db, DBTYPE type, uint32_t flags) {
    try {
        db = new Db(&m_env, 0);
        
        if (flags) {
            db->set_flags(flags);
        }
        
        db->open(NULL,
                dbName,
                NULL,
                type,
                DB_FLAGS,
                0);
    } catch (DbException &e) {
        std::cerr   << "Error opening database: "
                    << dbName << std::endl;
        std::cerr   << e.what() << std::endl;
        exit(-1);
    } catch (std::exception &e) {
        std::cerr   << "Error opening database: "
                    << dbName << std::endl;
        std::cerr   << e.what() << std::endl;
        exit(-1);
    }
}
