/*
 *  channel.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _CHANNEL_H_INCLUDED_
#define _CHANNEL_H_INCLUDED_

#include <map>
#include <vector>

class Client;
class Dispatcher;

class Channel {
    friend class Client;
    friend class Dispatcher;
    
public:
    Channel(char*);
    
    void subscribe(Client*);
    void unsubscribe(Client*);
    
    void broadcast(char*, int);
    
    ~Channel();
    
private:    
    char*                                   m_id;
    std::map<char*, Client*>                m_clients;
    
};

#endif
