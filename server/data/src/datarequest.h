/*
 *  datarequest.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _DATAREQUEST_H_INCLUDED_
#define _DATAREQUEST_H_INCLUDED_

#include "shared/request.h"

class DataRequest : public Request<DataRequest> {
public:
    DataRequest(int fd) : 
        Request<DataRequest>::Request(fd) {
    }
    
public:
    void dispatch();
    
public:
    void handleDisconnect() {}

};

#endif
