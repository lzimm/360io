/*
 *  conn.cpp
 *  360io
 *
 *  Created by Lewis Zimmerman on 09/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "conn.h"
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CONN_BUFFER_LEN     10240

Conn::Conn(char* host, int port, ConnPool* pool) :
    m_host(host),
    m_port(port),
    m_pool(pool),
    
    m_link(0),
    m_ctx(0),
    m_cb(0),
    
    m_input(new char[CONN_BUFFER_LEN]),
    m_inputUsed(0),
    m_inputLength(-1),
    
    m_output(0),
    m_outputUsed(0),
    m_outputLength(0),
    
    m_next(0),
    m_nextFree(0) {
    int reuseaddr_on        = 0;
    int conn_fd             = socket(AF_INET, SOCK_STREAM, 0); 
    
    if (conn_fd < 0) err(1, "listen failed");
    if (setsockopt(conn_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1) err(1, "setsockopt failed");
    
    sockaddr_in conn_addr;
    memset(&conn_addr, 0, sizeof(conn_addr));
    conn_addr.sin_family    = AF_INET;
    conn_addr.sin_port      = htons(port);
    
    struct hostent *hostinfo = gethostbyname(host);
    if (!hostinfo) err(1, "failed getting hostname");
    
    conn_addr.sin_addr      = *(struct in_addr *) hostinfo->h_addr;
    
    int flags               = fcntl(conn_fd, F_GETFL);
    if (flags < 0 || fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK) < 0) err(1, "failed to set client socket to non-blocking");
    
    m_fd                    = conn_fd;
    
    if (connect(conn_fd, reinterpret_cast<sockaddr*>(&conn_addr), sizeof(conn_addr)) != -1) err(1, "connect failed");
    
    m_read.set<Conn, &Conn::onRead>(this); 
    m_write.set<Conn, &Conn::onWrite>(this);
}

Conn::~Conn() {
    m_read.stop();
    m_write.stop();
    close(m_fd);
}


void Conn::cmd(char* cmd, int cmdLen, void* ctx, void (*cb)(void*, int, char*)) {
    m_ctx                   = ctx;
    m_cb                    = cb;

    m_output                = cmd;
    m_outputLength          = cmdLen;
    m_outputUsed            = 0;
    
    m_write.start(m_fd, ev::WRITE);
}

void Conn::onWrite(ev::io &w, int revents) {
    if (revents & EV_WRITE) {
        m_outputUsed        += write(m_fd, m_output + m_outputUsed, m_outputLength - m_outputUsed);

        if (m_outputUsed == m_outputLength) {
            m_write.stop();
            m_read.start(m_fd, ev::READ);
            
            delete[] m_output;
            m_output        = 0;
        }
    } else {
        return;
    }
}

void Conn::onRead(ev::io &w, int revents) {
    int bytesRead;
    
    if (revents & EV_READ) {
        bytesRead           = read(m_fd, m_input + m_inputUsed, CONN_BUFFER_LEN - m_inputUsed);
    } else {
        return;
    }
    
    if (bytesRead < 0) {        
        m_cb(m_ctx, -1, NULL);
        m_read.stop();

        m_inputUsed         = 0;
        m_inputLength       = -1;
        
        release();
        
        return;
    }
    
    int bytesAccumulated    = m_inputUsed + bytesRead;
    int dataLen             = m_inputLength;
    
    if (dataLen < 0) {
        char* p;
        for (p = m_input; p < m_input + bytesAccumulated; ++p) {
            if (*p == ' ') {
                *p          = '\0';
                break;
            }
        }
        
        if (p == m_input + bytesAccumulated) {
            return;
        }
    
        dataLen             = atoi(m_input);
    
        if (dataLen < 0) {        
            m_cb(m_ctx, -1, NULL);
            m_read.stop();

            m_inputUsed     = 0;
            m_inputLength   = -1;
        
            release();
            
            return;
        }
        
        bytesAccumulated    -= p - m_input;
        
        memmove(m_input, ++p, dataLen);
    }
    
    if (dataLen + 2 - bytesAccumulated > 0) {
        m_inputUsed         = bytesAccumulated;
        return;
    }
    
    *(m_input + dataLen)    = '\0';
    char* data              = new char[dataLen + 1];
    
    memcpy(data, m_input, dataLen + 1);

    m_cb(m_ctx, dataLen, data);
    m_read.stop();

    m_inputUsed             = 0;
    m_inputLength           = -1;
    
    release();
}

void Conn::release() {
    m_nextFree              = m_pool->m_free;
    m_pool->m_free          = this;
}

ConnPool::ConnPool(char* host, int port) :
    m_host(host),
    m_port(port),
    m_head(0),
    m_free(0) {
}

ConnPool::~ConnPool() {
    Conn* connection        = m_head;
    
    while (connection) {
        Conn* c             = connection;
        connection          = connection->m_next;
        
        delete c;  
    }
}

Conn* ConnPool::acquireConnection() {
    Conn* connection        = m_free;
    
    if (connection) {
        m_free              = connection->m_nextFree;
    } else {
        connection          = new Conn(m_host, m_port, this);
        
        connection->m_next  = m_head;
        m_head              = connection;
    }
    
    return connection;
}
