/*
 *  datastore.cpp
 *  360data
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "datastore.h"

#include <sstream>
#include <set>

#define DB_HEADIDXTYPE  DB_HASH
#define DB_TAILIDXTYPE  DB_BTREE

#define DB_HEADIDXFLAG  0
#define DB_TAILIDXFLAG  DB_DUPSORT

#define DB_LINKIDXTYPE  DB_HASH
#define DB_POSTIDXTYPE  DB_BTREE

#define DB_LINKIDXFLAG  0
#define DB_POSTIDXFLAG  DB_DUPSORT

#define DB_USERIDXTYPE  DB_HASH
#define DB_USERIDXFLAG  0

#define DB_HEADS        "headdb.db"
#define DB_TAILS        "taildb.db"

#define DB_LINKS        "linkdb.db"
#define DB_POSTS        "postdb.db"

#define DB_USERS        "userdb.db"

#define DATA_NULL       0x6C6C756E

void* DataStore::getUserPrivate(char* key) {
    char* userData      = static_cast<char*>(getUser(key));
    
    if (!userData) {
        return NULL;
    }
    
    char* prv           = static_cast<char*>(userData + 4);
    int prvLen          = *reinterpret_cast<int*>(userData);
    
    char* pub           = static_cast<char*>(userData + prvLen + 9);
    int pubLen          = *reinterpret_cast<int*>(userData + prvLen + 5);
    
    int fullLen         = m_privateUserTemplateLength + prvLen + pubLen;
    char* buff          = new char[fullLen + 1];
    *(buff + fullLen)   = '\0';
    
    sprintf(buff, m_privateUserTemplate, prv, pub);
    
    delete[] userData;
    
    return buff;
}

void* DataStore::getUserPublic(char* key) {
    char* userData      = static_cast<char*>(getUser(key));
    
    if (!userData) {
        return NULL;
    }

    int prvLen          = *reinterpret_cast<int*>(userData);
    
    char* pub           = static_cast<char*>(userData + prvLen + 9);
    int pubLen          = *reinterpret_cast<int*>(userData + prvLen + 5);
    
    char* buff          = new char[pubLen + 1];
    *(buff + pubLen) = '\0';
    
    strcpy(buff, pub);
    
    delete[] userData;
    
    return buff;
}

void DataStore::putUserPrivate(int len, char* key, void* data) {
    char* userData      = static_cast<char*>(getUser(key));
    
    if (!userData) {
        return;
    }
    
    int prvLen          = *reinterpret_cast<int*>(userData);
    int pubLen          = *reinterpret_cast<int*>(userData + prvLen + 5);

    int fLen            = pubLen + len + 10;
    char* buff          = new char[fLen];
    *(buff + fLen - 1)  = '\0';
    
    *reinterpret_cast<int*>(buff) = len;
    memcpy(buff + 4, data, len);
    *(buff + len + 4)   = '\0';
    
    memcpy(buff + len + 5, userData + prvLen + 5, pubLen + 4);
    
    putUser(fLen, key, buff);
    
    delete[] buff;
}

void DataStore::putUserPublic(int len, char* key, void* data) {
    char* userData      = static_cast<char*>(getUser(key));
    
    if (!userData) {
        return;
    }

    int prvLen          = *reinterpret_cast<int*>(userData);
    
    int fLen            = prvLen + len + 10;
    char* buff          = new char[fLen];
    *(buff + fLen - 1)  = '\0';
    
    memcpy(buff, userData, prvLen + 5);
    
    *reinterpret_cast<int*>(buff + prvLen + 5) = len;
    memcpy(buff + prvLen + 9, data, len);
    
    putUser(fLen, key, buff);
    
    delete[] buff;
}

void* DataStore::getLead(char* link) {
    char* linkData      = static_cast<char*>(getLink(link));
    
    if (!linkData) {
        return NULL;
    }
    
    char* title         = static_cast<char*>(linkData + 4);
    int titleLen        = *reinterpret_cast<int*>(linkData);
    
    char* url           = static_cast<char*>(linkData + titleLen + 9);
    int urlLen          = *reinterpret_cast<int*>(linkData + titleLen + 5);

    char* uid           = static_cast<char*>(linkData + titleLen + urlLen + 14);
    int uidLen          = *reinterpret_cast<int*>(linkData + titleLen + urlLen + 10);
    
    char* comment       = static_cast<char*>(linkData + titleLen + urlLen + uidLen + 19);
    int commentLen      = *reinterpret_cast<int*>(linkData + titleLen + urlLen + uidLen + 15);
    
    char* time          = static_cast<char*>(linkData + titleLen + urlLen + uidLen + commentLen + 24);
    int timeLen         = *reinterpret_cast<int*>(linkData + titleLen + urlLen + uidLen + commentLen + 20);
    
    char* user          = static_cast<char*>(linkData + titleLen + urlLen + uidLen + commentLen + timeLen + 29);
    int userLen         = *reinterpret_cast<int*>(linkData + titleLen + urlLen + uidLen + commentLen + timeLen + 25);
    
    if (FIRST_FOUR_BYTES(uid) != DATA_NULL) {
        if (char* tmp   = static_cast<char*>(getUserPublic(uid))) {
            user        = tmp;
            userLen     = strlen(user);
        }
    }
    
    int fullLen         = m_leadTemplateLength + titleLen + urlLen + uidLen + userLen + commentLen + timeLen;
    char* lead          = new char[fullLen + 1];
    *(lead + fullLen)   = '\0';
    
    sprintf(lead, m_leadTemplate, title, url, user, uid, comment, time);
    
    return lead;
}

void* DataStore::getStream(char* link) {
    std::stringstream stream;

    std::set<char*> uids;
    
    Dbc *cursor;
    m_postDB->cursor(NULL, &cursor, 0);
    
    Dbt key(link, strlen(link));
    Dbt data;
    
    stream << "{comments:[";
    
    int count           = 0;
    int ret             = cursor->get(&key, &data, DB_SET);
    while (ret != DB_NOTFOUND) {
        char* postData  = static_cast<char*>(data.get_data());

        char* id        = static_cast<char*>(postData + 4);
        int idLen       = *reinterpret_cast<int*>(postData);

        char* pid       = static_cast<char*>(postData + idLen + 9);
        int pidLen      = *reinterpret_cast<int*>(postData + idLen + 5);
           
        char* uid       = static_cast<char*>(postData + idLen + pidLen + 14);
        int uidLen      = *reinterpret_cast<int*>(postData + idLen + pidLen + 10);
        
        char* user      = static_cast<char*>(postData + idLen + pidLen + uidLen + 19);
        int userLen     = *reinterpret_cast<int*>(postData + idLen + pidLen + uidLen + 15);
        
        char* comment   = static_cast<char*>(postData + idLen + pidLen + uidLen + userLen + 24);
        int commentLen  = *reinterpret_cast<int*>(postData + idLen + pidLen + uidLen + userLen + 20);
        
        char* time      = static_cast<char*>(postData + idLen + pidLen + uidLen + userLen + commentLen + 29);
        int timeLen     = *reinterpret_cast<int*>(postData + idLen + pidLen + uidLen + userLen + commentLen + 25);
        
        int len         = m_streamCommentTemplateLength + idLen + pidLen + uidLen + userLen + commentLen + timeLen;
        char* post      = new char[len + 1];
        *(post + len)   = '\0';
        
        sprintf(post, m_streamCommentTemplate, id, pid, uid, user, comment, time);
        stream << (count++ > 0 ? "," : "") << post;
        
        if (FIRST_FOUR_BYTES(uid) != DATA_NULL) {        
            uids.insert(uid);
        }
        
        ret             = cursor->get(&key, &data, DB_NEXT_DUP);
    }
    
    if (cursor != NULL) cursor->close();
    
    stream << "], users:[";
    
    std::set<char*>::iterator iter;
    for (iter = uids.begin(); iter != uids.end(); iter++) {
        if (char* user = static_cast<char*>(getUserPublic(*iter))) {   
            stream << (iter == uids.begin() ? "" : ",") << user;
        }
    }
    
    stream << "]}";
    
    char* buff          = new char[stream.str().size() + 1];
    strcpy(buff, stream.str().c_str());

    return buff;
}

void* DataStore::putStream(int len, char* key, void* data) {
    putPost(len, key, data);
    
    char* postData  = static_cast<char*>(data);
    int idLen       = *reinterpret_cast<int*>(postData);
    char* uid       = static_cast<char*>(postData + idLen + 9);
    
    if (FIRST_FOUR_BYTES(uid) != DATA_NULL) {        
        return getUserPublic(uid);
    } else {
        return NULL;
    }
}

void* DataStore::getProperHead(char* key) {
    char* headData      = static_cast<char*>(getHead(key));
    
    if (!headData) {
        return NULL;
    }
    
    char* title         = static_cast<char*>(headData + 4);
    int titleLen        = *reinterpret_cast<int*>(headData);
    
    char* type          = static_cast<char*>(headData + titleLen + 9);
    int typeLen         = *reinterpret_cast<int*>(headData + titleLen + 5);

    char* uid           = static_cast<char*>(headData + titleLen + typeLen + 14);
    int uidLen          = *reinterpret_cast<int*>(headData + titleLen + typeLen + 10);
    
    char* params        = static_cast<char*>(headData + titleLen + typeLen + uidLen + 19);
    int paramLen        = *reinterpret_cast<int*>(headData + titleLen + typeLen + uidLen + 15);

    char* body          = static_cast<char*>(headData + titleLen + typeLen + uidLen + paramLen + 24);
    int bodyLen         = *reinterpret_cast<int*>(headData + titleLen + typeLen + uidLen + paramLen + 20);
    
    char* time          = static_cast<char*>(headData + titleLen + typeLen + uidLen + paramLen + bodyLen + 29);
    int timeLen         = *reinterpret_cast<int*>(headData + titleLen + typeLen + uidLen + paramLen + bodyLen + 25);
    
    char* user          = static_cast<char*>(headData + titleLen + typeLen + uidLen + paramLen + bodyLen + timeLen + 34);
    int userLen         = *reinterpret_cast<int*>(headData + titleLen + typeLen + uidLen + paramLen + bodyLen + timeLen + 30);
    
    if (FIRST_FOUR_BYTES(uid) != DATA_NULL) {
        if (char* tmp   = static_cast<char*>(getUserPublic(uid))) {
            user        = tmp;
            userLen     = strlen(user);
        }
    }
    
    int fullLen         = m_headTemplateLength + titleLen + typeLen + uidLen + userLen + paramLen + bodyLen + timeLen;
    char* head          = new char[fullLen + 1];
    *(head + fullLen)   = '\0';
    
    sprintf(head, m_headTemplate, title, type, user, uid, params, body, time);
    
    return head;
}

void* DataStore::getProperTail(char* ident) {
    std::stringstream stream;

    std::set<char*> uids;
    
    Dbc *cursor;
    m_tailDB->cursor(NULL, &cursor, 0);
    
    Dbt key(ident, strlen(ident));
    Dbt data;
    
    stream << "{tail:[";
    
    int count           = 0;
    int ret             = cursor->get(&key, &data, DB_SET);
    while (ret != DB_NOTFOUND) {
        char* tailData  = static_cast<char*>(data.get_data());

        char* id        = static_cast<char*>(tailData + 4);
        int idLen       = *reinterpret_cast<int*>(tailData);

        char* pid       = static_cast<char*>(tailData + idLen + 9);
        int pidLen      = *reinterpret_cast<int*>(tailData + idLen + 5);
           
        char* uid       = static_cast<char*>(tailData + idLen + pidLen + 14);
        int uidLen      = *reinterpret_cast<int*>(tailData + idLen + pidLen + 10);
        
        char* user      = static_cast<char*>(tailData + idLen + pidLen + uidLen + 19);
        int userLen     = *reinterpret_cast<int*>(tailData + idLen + pidLen + uidLen + 15);
        
        char* type      = static_cast<char*>(tailData + idLen + pidLen + uidLen + userLen + 24);
        int typeLen     = *reinterpret_cast<int*>(tailData + idLen + pidLen + uidLen + userLen + 20);

        char* params    = static_cast<char*>(tailData + idLen + pidLen + uidLen + userLen + typeLen + 29);
        int paramLen    = *reinterpret_cast<int*>(tailData + idLen + pidLen + uidLen + userLen + typeLen + 25);

        char* body      = static_cast<char*>(tailData + idLen + pidLen + uidLen + userLen + typeLen + paramLen + 34);
        int bodyLen     = *reinterpret_cast<int*>(tailData + idLen + pidLen + uidLen + userLen + typeLen + paramLen + 30);
        
        char* time      = static_cast<char*>(tailData + idLen + pidLen + uidLen + userLen + typeLen + paramLen + bodyLen + 39);
        int timeLen     = *reinterpret_cast<int*>(tailData + idLen + pidLen + uidLen + userLen + typeLen + paramLen + bodyLen + 35);
        
        int len         = m_tailEntryTemplateLength + idLen + pidLen + uidLen + userLen + typeLen + paramLen + bodyLen + timeLen;
        char* tail      = new char[len + 1];
        *(tail + len)   = '\0';
        
        sprintf(tail, m_tailEntryTemplate, id, pid, uid, user, type, params, body, time);
        stream << (count++ > 0 ? "," : "") << tail;
        
        if (FIRST_FOUR_BYTES(uid) != DATA_NULL) {        
            uids.insert(uid);
        }
        
        ret             = cursor->get(&key, &data, DB_NEXT_DUP);
    }
    
    if (cursor != NULL) cursor->close();
    
    stream << "], users:[";
    
    std::set<char*>::iterator iter;
    for (iter = uids.begin(); iter != uids.end(); iter++) {
        if (char* udata = static_cast<char*>(getUserPublic(*iter))) {   
            stream << (iter == uids.begin() ? "" : ",") << udata;
        }
    }
    
    stream << "]}";
    
    char* buff          = new char[stream.str().size() + 1];
    strcpy(buff, stream.str().c_str());

    return buff;
}

void* DataStore::putProperTail(int len, char* key, void* data) {
    putTail(len, key, data);
    
    char* tailData  = static_cast<char*>(data);
    int idLen       = *reinterpret_cast<int*>(tailData);
    int pidLen      = *reinterpret_cast<int*>(tailData + idLen + 5);
    char* uid       = static_cast<char*>(tailData + idLen + pidLen + 14);
    
    if (FIRST_FOUR_BYTES(uid) != DATA_NULL) {        
        return getUserPublic(uid);
    } else {
        return NULL;
    }
}

DataStore::DataStore() : Store::Store() {
    try {
        initDB(DB_HEADS, m_headDB, DB_HEADIDXTYPE, DB_HEADIDXFLAG);
        initDB(DB_TAILS, m_tailDB, DB_TAILIDXTYPE, DB_TAILIDXFLAG);
        
        initDB(DB_LINKS, m_linkDB, DB_LINKIDXTYPE, DB_LINKIDXFLAG);
        initDB(DB_POSTS, m_postDB, DB_POSTIDXTYPE, DB_POSTIDXFLAG);
        
        initDB(DB_USERS, m_userDB, DB_USERIDXTYPE, DB_USERIDXFLAG);
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

    m_headTemplate                  = "{title:\"%s\", type:\"%s\", user:%s, head:{uid:\"%s\", params:%s, body:\"%s\", time:\"%s\"}}";
    m_tailEntryTemplate             = "{id:\"%s\", parent:\"%s\", uid:\"%s\", user:%s, type:\"%s\", params:%s, body:\"%s\", time:\"%s\"}";

    m_headTemplateLength            = strlen(m_headTemplate) - 14;
    m_tailEntryTemplateLength       = strlen(m_tailEntryTemplate) - 16;
        
    m_leadTemplate                  = "{title:\"%s\", url:\"%s\", user:%s, lead:{uid:\"%s\", body:\"%s\", time:\"%s\"}}";
    m_streamCommentTemplate         = "{id:\"%s\", parent:\"%s\", uid:\"%s\", user:%s, body:\"%s\", time:\"%s\"}";
    
    m_leadTemplateLength            = strlen(m_leadTemplate) - 12;
    m_streamCommentTemplateLength   = strlen(m_streamCommentTemplate) - 12;
    
    m_privateUserTemplate           = "{\"private\":%s, \"public\":%s}";
    m_privateUserTemplateLength     = strlen(m_privateUserTemplate) - 2;
}

DataStore::~DataStore() {
    m_headDB->close(0);
    m_tailDB->close(0);

    delete m_headDB;
    delete m_tailDB;
    
    m_linkDB->close(0);
    m_postDB->close(0);
    
    delete m_linkDB;
    delete m_postDB;
    
    m_userDB->close(0);
    delete m_userDB;
}
