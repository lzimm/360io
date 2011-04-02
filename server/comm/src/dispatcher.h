/*
 *  dispatcher.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _DISPATCHER_H_INCLUDED_
#define _DISPATCHER_H_INCLUDED_

#include <map>
#include <string>

#include "commrequest.h"
#include "client.h"
#include "channel.h"

class Dispatcher {
public:
    Dispatcher() {}
    ~Dispatcher() {}

public:
    void eraseClient(char*);
    void eraseChannel(char*);
    
public:
    char* create(char*);
    int attach(char*, CommRequest*);
    int subscribe(char*, char*);
    int unsubscribe(char*, char*);
    int broadcast(char*, char*, int);
    int detach(char*, CommRequest*);

private:
    std::map<std::string, Client*>     m_clients;
    std::map<std::string, Channel*>    m_channels;
    
};

#endif
