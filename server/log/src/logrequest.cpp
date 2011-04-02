/*
 *  logrequest.cpp
 *  360log
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "logrequest.h"
#include "manager.h"
#include "logstatic.h"

#define DISPATCH_BRG                0x2F475242
#define DISPATCH_CHD                0x2F444843
#define DEFAULT_RETURN_LEN          25

void LogRequest::dispatch() {
    switch (FIRST_FOUR_BYTES(m_path + 1)) {        
        case DISPATCH_BRG:
            m_staticOutput = LogStatic::s_bridgeTemplate;
        break;
    
        case DISPATCH_CHD:
            m_staticOutput = LogStatic::s_childTemplate;
        break;
        
        default:
            m_output = static_cast<char*>(Manager::store.pull(m_path + 1, DEFAULT_RETURN_LEN));
        break;
    }
    
    m_write.start(m_fd, ev::WRITE);
}
