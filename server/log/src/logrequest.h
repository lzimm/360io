/*
 *  logrequest.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _LOGREQUEST_H_INCLUDED_
#define _LOGREQUEST_H_INCLUDED_

#include "shared/request.h"

class LogRequest : public Request<LogRequest> {
public:
    LogRequest(int fd) : 
        Request<LogRequest>::Request(fd) {
    }
    
public:
    void dispatch();
    
public:
    void handleDisconnect() {}

};

#endif
