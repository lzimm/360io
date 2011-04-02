/*
 *  client.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _CLIENT_H_INCLUDED_
#define _CLIENT_H_INCLUDED_

#include <map>

#include <ev++.h>

class Channel;
class CommRequest;
class Dispatcher;

class Client {
    friend class Channel;
    friend class Dispatcher;
    
public:
    Client(char*, char*);
    
    void subscribe(Channel*);
    void unsubscribe(Channel*);
    
    void connect(CommRequest*);
    void disconnect(CommRequest*);
    void disconnect();
    
    void send(char*, int len);
    
    void release();
    void clearTimeout();
    void resetTimeout();

    ~Client();
    
private:
    void onTimerCheck(ev::timer&, int);
    void onTimerDisconnect(ev::timer&, int);
    
private:    
    char*                                   m_id;
    char*                                   m_ip;

    CommRequest*                            m_request;

    char*                                   m_queue;
    char*                                   m_queueOffset;
    int                                     m_queueSize;
    
    ev::timer                               m_timer;
    
    std::map<char*, Channel*>               m_subscriptions;
    
};

#endif
