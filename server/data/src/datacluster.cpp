/*
 *  datacluster.cpp
 *  360data
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "datacluster.h"
#include "manager.h"

#define DISPATCH_RETR   0x52544552
#define DISPATCH_REPL   0x4C504552
#define DISPATCH_APND   0x444E5041
#define DISPATCH_STOR   0x524F5453

#define DISPATCH_HEAD   0x44414548
#define DISPATCH_TAIL   0x4C494154
#define DISPATCH_LINK   0x4B4E494C
#define DISPATCH_POST   0x54534F50
#define DISPATCH_AUTH   0x48545541
#define DISPATCH_USER   0x52455355
#define DISPATCH_LEAD   0x4441454C
#define DISPATCH_STRM   0x4D525453

void DataClusterConnection::dispatch() {
    switch (FIRST_FOUR_BYTES(m_command)) {
        case DISPATCH_STOR:
            *(m_commandBody + m_commandLength) = '\0';
            
            bool res;
            switch (FIRST_FOUR_BYTES(m_command + 5)) {
                case DISPATCH_HEAD:
                    res = Manager::store.addHead(m_commandLength + 1, m_commandParam, m_commandBody);
                    break;
                    
                case DISPATCH_TAIL:
                    res = Manager::store.addTail(m_commandLength + 1, m_commandParam, m_commandBody);
                    break;
                    
                case DISPATCH_LINK:
                    res = Manager::store.addLink(m_commandLength + 1, m_commandParam, m_commandBody);
                    break;
                    
                case DISPATCH_POST:
                    res = Manager::store.addPost(m_commandLength + 1, m_commandParam, m_commandBody);
                    break;
                    
                case DISPATCH_USER:
                    res = Manager::store.addUser(m_commandLength + 1, m_commandParam, m_commandBody);
                    break;
            }
            
            if (res) {
                m_output = new char[3];
                memcpy(m_output, "ok", 3);
            } else {
                m_output = new char[6];
                memcpy(m_output, "error", 6);
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
            
        case DISPATCH_APND:
            *(m_commandBody + m_commandLength) = '\0';
            
            switch (FIRST_FOUR_BYTES(m_command + 5)) {
                case DISPATCH_HEAD:
                    Manager::store.appendHead(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                
                case DISPATCH_TAIL:
                    Manager::store.appendTail(m_commandLength + 1, m_commandParam, m_commandBody);
                break;

                case DISPATCH_LINK:
                    Manager::store.appendLink(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                    
                case DISPATCH_POST:
                    Manager::store.appendPost(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                    
                case DISPATCH_USER:
                    Manager::store.appendUser(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
            
        case DISPATCH_REPL:
            *(m_commandBody + m_commandLength) = '\0';
            
            switch (FIRST_FOUR_BYTES(m_command + 5)) {
                case DISPATCH_HEAD:
                    Manager::store.putHead(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                
                case DISPATCH_TAIL:
                    m_output = static_cast<char*>(Manager::store.putProperTail(m_commandLength + 1, m_commandParam, m_commandBody));
                break;
                
                case DISPATCH_LINK:
                    Manager::store.putLink(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                
                case DISPATCH_POST:
                    Manager::store.putPost(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                    
                case DISPATCH_AUTH:
                    Manager::store.putUserPrivate(m_commandLength + 1, m_commandParam, m_commandBody);
                break;

                case DISPATCH_USER:
                    Manager::store.putUserPublic(m_commandLength + 1, m_commandParam, m_commandBody);
                break;
                
                case DISPATCH_STRM:
                    m_output = static_cast<char*>(Manager::store.putStream(m_commandLength + 1, m_commandParam, m_commandBody));
                break;
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
        
        case DISPATCH_RETR:
            switch (FIRST_FOUR_BYTES(m_command + 5)) {
                case DISPATCH_HEAD:
                    m_output = static_cast<char*>(Manager::store.getProperHead(m_commandParam));
                break;
                
                case DISPATCH_TAIL:
                    m_output = static_cast<char*>(Manager::store.getProperTail(m_commandParam));
                break;
                
                case DISPATCH_LINK:
                    m_output = static_cast<char*>(Manager::store.getLink(m_commandParam));
                break;
                    
                case DISPATCH_POST:
                    m_output = static_cast<char*>(Manager::store.getPost(m_commandParam));
                break;
                    
                case DISPATCH_AUTH:
                    m_output = static_cast<char*>(Manager::store.getUserPrivate(m_commandParam));
                break;

                case DISPATCH_USER:
                    m_output = static_cast<char*>(Manager::store.getUserPublic(m_commandParam));
                break;
                    
                case DISPATCH_LEAD:
                    m_output = static_cast<char*>(Manager::store.getLead(m_commandParam));
                break;
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
            
        default:
            m_read.start(m_fd, ev::READ);
        break;
    }
}
