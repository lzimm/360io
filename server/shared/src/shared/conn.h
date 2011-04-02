/*
 *  conn.h
 *  360io
 *
 *  Created by Lewis Zimmerman on 09/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _CONN_H_INCLUDED_
#define _CONN_H_INCLUDED_

#include <ev++.h>

class ConnPool;

class Conn {
    friend class ConnPool;
    
public:
    Conn(char*, int, ConnPool*);
    ~Conn();

public:
    void cmd(char*, int, void*, void (*)(void*, int, char*));
    
public:
    void release();
    
private:
    void onRead(ev::io&, int );
    void onWrite(ev::io&, int); 
    
private:
    char*               m_host;
    int                 m_port;
    
    int                 m_fd;
    
    ev::io              m_read;
    ev::io              m_write;
    
    ConnPool*           m_pool;
    
    char*               m_link;
    void*               m_ctx;
    void                (*m_cb)(void*, int, char*);
    
    char*               m_input;
    int                 m_inputUsed;
    int                 m_inputLength;
    
    char*               m_output;
    int                 m_outputUsed;
    int                 m_outputLength;
    
    Conn*               m_next;
    Conn*               m_nextFree;
    
};

class ConnPool {
    friend class Conn;
    
public:
    ConnPool(char*, int);
    ~ConnPool();
    
public:
    Conn* acquireConnection();

private:
    char*               m_host;
    int                 m_port;
    
    Conn*               m_head;
    Conn*               m_free;

};

#endif
