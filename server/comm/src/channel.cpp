/*
 *  channel.cpp
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "channel.h"
#include "client.h"
#include "manager.h"

Channel::Channel(char* id) {
    int idLen = strlen(id) + 1;
    m_id = new char[idLen];
    memcpy(m_id, id, idLen);
    
    m_clients = std::map<char*, Client*>();
}

void Channel::subscribe(Client* client) {
    m_clients[client->m_id] = client;
    client->m_subscriptions[m_id] = this;
}

void Channel::unsubscribe(Client* client) {
    m_clients.erase(client->m_id);
    client->m_subscriptions.erase(m_id);
    
    if (m_clients.size() == 0) {
        delete this;
    }
}

void Channel::broadcast(char* message, int len) {
    if (m_clients.size() == 0) return;
    
    std::map<char*, Client*>::const_iterator itr;
    for (itr = m_clients.begin(); itr != m_clients.end(); itr++) {
        itr->second->send(message, len);
    }    
}

Channel::~Channel() {
    Manager::dispatcher.eraseChannel(m_id);
    
    std::map<char*, Client*>::const_iterator itr;
    for (itr = m_clients.begin(); itr != m_clients.end(); itr++) {
        itr->second->m_subscriptions.erase(m_id);
    }
    
    delete[] m_id;
}
