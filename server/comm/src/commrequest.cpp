/*
 *  commrequest.cpp
 *  360comm
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "commrequest.h"
#include "manager.h"
#include "commstatic.h"

#define DISPATCH_NEW                0x57454E2F
#define DISPATCH_LTN                0x4E544C2F
#define DISPATCH_ASB                0x4253412F
#define DISPATCH_RSB                0x4253522F

#define DISPATCH_BRG                0x4752422F
#define DISPATCH_CHD                0x4448432F
#define DISPATCH_CRO                0x6F72632F

#define CID_LENGTH                  36

void CommRequest::send(char* payload) {
    int len = strlen(payload);
    
    memcpy(m_outputOffset, payload, len);
    m_outputOffset += len;
    
    *m_outputOffset = '\0';
}

void CommRequest::complete() {
    if ((m_outputOffset == m_output)) {
        if (m_write.active) {
            m_write.stop();
        }
        
        delete this;
    } else {
        if (m_fd > 0) {
            m_write.start(m_fd, ev::WRITE);
        }
    }
}

void CommRequest::dispatch() {
    int res;
    char* cid;
    switch (FIRST_FOUR_BYTES(m_path)) {
        case DISPATCH_NEW:
            cid = Manager::dispatcher.create("127.0.0.1");
            
            send(cid);
            complete();
            
            return;
        break;
        
        case DISPATCH_LTN:
            //*(m_host + CID_LENGTH) = '\0';
            res = Manager::dispatcher.attach(m_path + 5, this);
            
            if (!res) {
                send("error");
                complete();
            } else {
                m_cid = m_path + 5;
            }
        break;
            
        case DISPATCH_ASB:
            //*(m_host + CID_LENGTH) = '\0';
            res = Manager::dispatcher.subscribe(m_path + 5, m_path + CID_LENGTH + 6);
            
            if (res) {
                send("success");
            } else {
                send("error");
            }
            
            complete();
        break;
            
        case DISPATCH_RSB:
            //*(m_host + CID_LENGTH) = '\0';
            res = Manager::dispatcher.unsubscribe(m_path + 5, m_path + CID_LENGTH + 6);
            
            if (res) {
                send("success");
            } else {
                send("error");
            }
            
            complete();
        break;
            
        case DISPATCH_BRG:
            m_staticOutput = CommStatic::s_bridgeTemplate;
            m_write.start(m_fd, ev::WRITE);
        break;
            
        case DISPATCH_CHD:
            m_staticOutput = CommStatic::s_childTemplate;
            m_write.start(m_fd, ev::WRITE);
        break;
        
        case DISPATCH_CRO:
            m_staticOutput = CommStatic::s_crossDomainXML;
            m_write.start(m_fd, ev::WRITE);
        break;
            
        default:
            send("error");
            complete();
        break;
    }
}

void CommRequest::handleDisconnect() {
    if (m_cid) {
        Manager::dispatcher.detach(m_cid, this);
    }
}
