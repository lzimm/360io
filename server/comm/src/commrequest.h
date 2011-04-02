/*
 *  commrequest.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _COMMREQUEST_H_INCLUDED_
#define _COMMREQUEST_H_INCLUDED_

#include "shared/request.h"

class CommRequest : public Request<CommRequest> {
public:
    CommRequest(int fd) : 
        Request<CommRequest>::Request(fd),
        m_cid(0) {
        m_output = new char[OUTPUT_BUFFER_LEN];
        m_outputOffset = m_output;
    }
    
public:
    void send(char*);
    void complete();
    
public:
    void dispatch();
    
public:
    void handleDisconnect();
    
private:
    char*       m_outputOffset;
    char*       m_cid;

};

#endif
