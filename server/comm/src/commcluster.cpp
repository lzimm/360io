/*
 *  commcluster.cpp
 *  360comm
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "commcluster.h"
#include "manager.h"

#include "iostream"

#define DISPATCH_INIT   0x54494E49
#define DISPATCH_ASUB   0x42555341
#define DISPATCH_RSUB   0x42555352
#define DISPATCH_SEND   0x444E4553

void CommClusterConnection::dispatch() {
    switch (FIRST_FOUR_BYTES(m_command)) {
        case DISPATCH_INIT:
            m_output = Manager::dispatcher.create(m_commandParam);
        break;
        
        case DISPATCH_ASUB:
            Manager::dispatcher.subscribe(m_commandParam, m_commandBody);
        break;
            
        case DISPATCH_RSUB:
            Manager::dispatcher.unsubscribe(m_commandParam, m_commandBody);
        break;
            
        case DISPATCH_SEND:
            Manager::dispatcher.broadcast(m_commandParam, m_commandBody, m_commandLength);
        break;
    }
    
    m_write.start(m_fd, ev::WRITE);
}
