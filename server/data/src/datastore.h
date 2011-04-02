/*
 *  datastore.h
 *  360data
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _DATASTORE_H_INCLUDED_
#define _DATASTORE_H_INCLUDED_

#include "shared/store.h"
#include "shared/util.h"

class DataStore : public Store {
public:
    DataStore();
    ~DataStore();
    
public:
    void* getUserPrivate(char*);
    void* getUserPublic(char*);
    
    void putUserPrivate(int, char*, void*);
    void putUserPublic(int, char*, void*);

    void* getLead(char*);
    void* getStream(char*);
    void* putStream(int, char*, void*);
    
    void* getProperHead(char*);
    void* getProperTail(char*);
    void* putProperTail(int, char*, void*);

public:
    DEFINE_DB_INTERFACE(Head, m_headDB)
    DEFINE_DB_INTERFACE(Tail, m_tailDB)
    
    DEFINE_DB_INTERFACE(Link, m_linkDB)
    DEFINE_DB_INTERFACE(Post, m_postDB)
    
    DEFINE_DB_INTERFACE(User, m_userDB)

private:
    Db*         m_headDB;
    Db*         m_tailDB;
    
    Db*         m_linkDB;
    Db*         m_postDB;
    
    Db*         m_userDB;
    
    int         m_headTemplateLength;
    int         m_tailEntryTemplateLength;
    
    int         m_leadTemplateLength;
    int         m_streamCommentTemplateLength;
    
    int         m_privateUserTemplateLength;
    
    char*       m_headTemplate;
    char*       m_tailEntryTemplate;
    
    char*       m_leadTemplate;
    char*       m_streamCommentTemplate;
    
    char*       m_privateUserTemplate;
    
};

#endif
