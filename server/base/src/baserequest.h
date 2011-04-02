/*
 *  baserequest.h
 *  360base
 *
 *  Created by Lewis Zimmerman on 16/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _BASEREQUEST_H_INCLUDED_
#define _BASEREQUEST_H_INCLUDED_

#include "shared/request.h"

class BaseRequest : public Request<BaseRequest> {
public:
    BaseRequest(int fd) : 
        Request<BaseRequest>::Request(fd),
        
        m_target(0),
        
        m_commIdent(0),
        m_dataHead(0),
        m_dataTail(0),
        
        m_commIdentLength(0),
        m_dataHeadLength(0),
        m_dataTailLength(0) {
    };
    
public:
    void sendLanding();
    void sendStream(char*);
    void sendStream();
    void dispatch();

public:
    void handleDisconnect() {}
    
public:
    static void onDataHead(void* ctx, int dataLength, char* data);
    static void onDataTail(void* ctx, int dataLength, char* data);
    static void onCommInit(void* ctx, int dataLength, char* data);
    static void onCommAsub(void* ctx, int dataLength, char* data);
    
private:
    char*   m_target;
    
    char*   m_commIdent;    
    char*   m_dataHead;
    char*   m_dataTail;
    
    int     m_commIdentLength;
    int     m_dataHeadLength;
    int     m_dataTailLength;

};

#endif
