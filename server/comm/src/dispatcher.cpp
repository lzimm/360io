/*
 *  dispatcher.cpp
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "dispatcher.h"

#include <uuid/uuid.h>

#define UPPER_TO_LOWER      32

void Dispatcher::eraseClient(char* cid) {
    m_clients.erase(cid);
}

void Dispatcher::eraseChannel(char* name) {
    m_channels.erase(name);
}

char* Dispatcher::create(char* ip) {
    uuid_t uuid;
    uuid_generate_random(uuid);
    
    char* cid = new char[37];
    uuid_unparse(uuid, cid);
    
    char ch;
    for (int i = 0; i < 36; ++i) {
        ch = cid[i];
        if (ch >= 'A' && ch <= 'Z') {
            cid[i] = ch + UPPER_TO_LOWER;
        }
    }
    
    Client* client = new Client(cid, ip);
    m_clients[client->m_id] = client;
    
    return cid;
}

int Dispatcher::attach(char* cid, CommRequest* req) {
    Client* client = m_clients[cid];
    
    if (!client) return 0;
    
    client->connect(req);
    return 1;
}

int Dispatcher::subscribe(char* cid, char* name) {
    Client* client = m_clients[cid];

    if (!client) return 0;
    
    Channel* channel = m_channels[name];
    
    if (!channel) {
        channel = new Channel(name);
        m_channels[channel->m_id] = channel;
    }
    
    channel->subscribe(client);
    return 1;
}

int Dispatcher::unsubscribe(char* cid, char* name) {
    Client* client = m_clients[cid];
    Channel* channel = m_channels[name];
    
    if (!client || !channel) return 0;
    
    channel->unsubscribe(client);
    return 1;
}

int Dispatcher::broadcast(char* name, char* message, int len) {
    Channel* channel = m_channels[name];
    
    if (!channel) return 0;
    
    channel->broadcast(message, len);
    return 1;    
}

int Dispatcher::detach(char* cid, CommRequest* req) {
    Client* client = m_clients[cid];
    
    if (!client) return 0;
    
    client->disconnect(req);
    return 1;
}
