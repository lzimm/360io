/*
 *  client.cpp
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "client.h"
#include "channel.h"
#include "commrequest.h"
#include "manager.h"

#define CONNECTED_TIMEOUT       60
#define DISCONNECTED_TIMEOUT    5

#define QUEUE_BUFFER_LEN        10240
#define QUEUE_BUFFER_MULTIPLE   2

Client::Client(char* id, char* ip) : 
    m_request(NULL), 
    m_queue(new char[QUEUE_BUFFER_LEN]), 
    m_queueOffset(m_queue),
    m_queueSize(QUEUE_BUFFER_LEN) {
    int idLen = strlen(id) + 1;
    m_id = new char[idLen];
    memcpy(m_id, id, idLen);
        
    int ipLen = strlen(ip) + 1;
    m_ip = new char[ipLen];
    memcpy(m_ip, ip, ipLen);

    m_subscriptions     = std::map<char*, Channel*>();
    
    m_timer.set<Client, &Client::onTimerCheck>(this);
    m_timer.repeat = CONNECTED_TIMEOUT;
    m_timer.again();
}

void Client::onTimerCheck(ev::timer &w, int revents) {
    if (m_request) {
        m_request->send("reconnect");
        m_request->complete();
        m_request = NULL;
    }
    
    m_timer.set<Client, &Client::onTimerDisconnect>(this);
    m_timer.repeat = DISCONNECTED_TIMEOUT;
    m_timer.again();
}

void Client::onTimerDisconnect(ev::timer &w, int revents) {
    m_timer.stop();
    delete this;
}

void Client::subscribe(Channel* channel) {
    m_subscriptions[channel->m_id] = channel;
    channel->m_clients[m_id] = this;
}

void Client::unsubscribe(Channel* channel) {
    m_subscriptions.erase(channel->m_id);
    channel->m_clients.erase(m_id);
    
    if (channel->m_clients.size() == 0) {
        delete channel;
    }
}

void Client::connect(CommRequest* req) {
    if (m_queueOffset > m_queue) {
        req->send(m_queue);
        req->complete();
        
        m_queueOffset = m_queue;
    } else {
        m_request = req;
    }
    
    resetTimeout();
}

void Client::disconnect(CommRequest* req) {
    if (m_request == req) {
        m_request = 0;
        resetTimeout();
    }
}

void Client::disconnect() {
    resetTimeout();
}

void Client::send(char* message, int len) {
    if (m_request) {
        m_request->send(message);
        m_request->complete();
        m_request = NULL;
        
        resetTimeout();
    } else {
        int curQueueLen = m_queueOffset - m_queue; 
        
        if (curQueueLen + len > m_queueSize) {
            m_queueSize *= QUEUE_BUFFER_MULTIPLE;
            char* newQueue = new char[m_queueSize];
            
            memcpy(newQueue, m_queue, curQueueLen);
            delete[] m_queue;
            
            m_queue = newQueue;
            m_queueOffset = m_queue + curQueueLen;
        }
        
        memcpy(m_queueOffset, message, len);
        
        m_queueOffset += len;
        *m_queueOffset = '\0';
    }
}

void Client::release() {
    m_request->send("release");
    m_request->complete();
    m_request = NULL;
    
    resetTimeout();
}

void Client::clearTimeout() {
    m_timer.stop();
}

void Client::resetTimeout() {
    m_timer.stop();
    m_timer.set<Client, &Client::onTimerCheck>(this);
    m_timer.start(CONNECTED_TIMEOUT);
}

Client::~Client() {    
    Manager::dispatcher.eraseClient(m_id);
    
    std::map<char*, Channel*>::const_iterator itr;
    for (itr = m_subscriptions.begin(); itr != m_subscriptions.end(); itr++) {
        itr->second->m_clients.erase(m_id);
    }
    
    if (m_request) {
        m_request->send("cleanup");
        m_request->complete();
    }
    
    delete[] m_id;
}
