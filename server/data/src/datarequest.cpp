/*
 *  request.cpp
 *  360data
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "datarequest.h"
#include "manager.h"
#include "datastatic.h"

#define DISPATCH_HEAD               0x44414548
#define DISPATCH_TAIL               0x4C494154
#define DISPATCH_LINK               0x4B4E494C
#define DISPATCH_POST               0x54534F50
#define DISPATCH_AUTH               0x48545541
#define DISPATCH_USER               0x52455355
#define DISPATCH_STRM               0x4D525453

#define DISPATCH_BRG                0x2F475242
#define DISPATCH_CHD                0x2F444843
#define DISPATCH_CRO                0x736F7263

void DataRequest::dispatch() {
    switch (FIRST_FOUR_BYTES(m_path + 1)) {
        case DISPATCH_LINK:
            m_output = static_cast<char*>(Manager::store.getLink(m_path + 6));
        break;
            
        case DISPATCH_POST:
            m_output = static_cast<char*>(Manager::store.getPost(m_path + 6));
        break;
            
        case DISPATCH_AUTH:
            m_output = static_cast<char*>(Manager::store.getUserPrivate(m_path + 6));
        break;

        case DISPATCH_USER:
            m_output = static_cast<char*>(Manager::store.getUserPublic(m_path + 6));
        break;
            
        case DISPATCH_STRM:
            m_output = static_cast<char*>(Manager::store.getStream(m_path + 6));
        break;
        
        case DISPATCH_HEAD:
            m_output = static_cast<char*>(Manager::store.getProperHead(m_path + 6));
        break;
        
        case DISPATCH_TAIL:
            m_output = static_cast<char*>(Manager::store.getProperTail(m_path + 6));
        break;
        
        case DISPATCH_BRG:
            m_staticOutput = DataStatic::s_bridgeTemplate;
        break;
    
        case DISPATCH_CHD:
            m_staticOutput = DataStatic::s_childTemplate;
        break;
        
        case DISPATCH_CRO:
            m_staticOutput = DataStatic::s_crossDomainXML;
        break;
    }
    
    m_write.start(m_fd, ev::WRITE);
}
