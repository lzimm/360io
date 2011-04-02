/*
 *  syncrequest.h
 *  360sync
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _SYNCREQUEST_H_INCLUDED_
#define _SYNCREQUEST_H_INCLUDED_

#include "shared/request.h"

class SyncRequest : public Request<SyncRequest> {
public:
    SyncRequest(int fd) : 
        Request<SyncRequest>::Request(fd) {
    }
    
public:
    void dispatch();
    
public:
    void handleDisconnect() {}
    
};

#endif
