/*
 *  logcluster.cpp
 *  360log
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "logcluster.h"
#include "manager.h"

#define DISPATCH_PULL   0x4C4C5550
#define DISPATCH_PUSH   0x48535550

void LogClusterConnection::dispatch() {
    switch (FIRST_FOUR_BYTES(m_command)) {
        case DISPATCH_PUSH:
            *(m_commandBody + m_commandLength) = '\0';

            if (Manager::store.push(m_commandLength + 1, m_commandParam, m_commandBody)) {
                m_output = new char[3];
                memcpy(m_output, "ok", 3);
            } else {
                m_output = new char[6];
                memcpy(m_output, "error", 6);
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
        
        case DISPATCH_PULL:
            m_output = static_cast<char*>(Manager::store.pull(m_commandParam, *(reinterpret_cast<uint32_t*>(m_commandBody))));
            
            m_write.start(m_fd, ev::WRITE);
        break;
            
        default:
            m_read.start(m_fd, ev::READ);
        break;
    }
}
