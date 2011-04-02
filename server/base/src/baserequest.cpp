/*
 *  baserequest.cpp
 *  360io
 *
 *  Created by Lewis Zimmerman on 16/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "baserequest.h"
#include "basestatic.h"
#include "manager.h"
#include "shared/util.h"

void BaseRequest::sendLanding() {
    m_staticOutput              = BaseStatic::s_landingTemplate;
    m_write.start(m_fd, ev::WRITE);
}

void BaseRequest::sendStream(char* id) {    
    int idLen                   = strlen(id);
    m_target                    = new char[idLen + 1];

    strcpy(m_target, id);
    *(m_target + idLen)         = '\0';
    
    int headCmdLen              = 15 + idLen;
    char* headCmd               = new char[headCmdLen + 1];

    sprintf(headCmd, "0 RETR_HEAD %s \r\n", id);
    
    Manager::acquireDataConnection()->cmd(headCmd, headCmdLen, this, onDataHead); 
}

void BaseRequest::onDataHead(void* ctx, int dataLength, char* data) {
    BaseRequest* request        = static_cast<BaseRequest*>(ctx);
    
    if (data) {
        request->m_dataHead         = data;
        request->m_dataHeadLength   = dataLength;
    
        int initCmdLen              = 10 + strlen(request->m_target);
        char* initCmd               = new char[initCmdLen + 1];

        sprintf(initCmd, "0 INIT %s \r\n", request->m_target);
        Manager::acquireCommConnection()->cmd(initCmd, initCmdLen, request, onCommInit); 
    } else {
        request->sendStream();
    }
}

void BaseRequest::onCommInit(void* ctx, int dataLength, char* data) {
    BaseRequest* request        = static_cast<BaseRequest*>(ctx);
    
    request->m_commIdent        = data;
    request->m_commIdentLength  = dataLength;
    
    int targetLen               = strlen(request->m_target);
    int channelLen              = targetLen + 11;
    int asubCmdLen              = 21 + Util::intWidth(channelLen) + dataLength + targetLen;
    char* asubCmd               = new char[asubCmdLen + 1];
    sprintf(asubCmd, "%i ASUB %s /chan/tail_%s \r\n", channelLen, data, request->m_target);
    
    Manager::acquireCommConnection()->cmd(asubCmd, asubCmdLen, request, onCommAsub);
}

void BaseRequest::onCommAsub(void* ctx, int dataLength, char* data) {
    BaseRequest* request        = static_cast<BaseRequest*>(ctx);

    int tailCmdLen              = 15 + strlen(request->m_target);
    char* tailCmd               = new char[tailCmdLen + 1];
    sprintf(tailCmd, "0 RETR_TAIL %s \r\n", request->m_target);
    
    if (data) delete[] data;

    Manager::acquireDataConnection()->cmd(tailCmd, tailCmdLen, request, onDataTail);    
}

void BaseRequest::onDataTail(void* ctx, int dataLength, char* data) {
    BaseRequest* request        = static_cast<BaseRequest*>(ctx);
    
    request->m_dataTail         = data;
    request->m_dataTailLength   = dataLength;
    
    request->sendStream();
}

void BaseRequest::sendStream() {
    if (m_dataHeadLength > 0) {
        int contentLength           = BaseStatic::s_adjustedStreamTemplateLength + m_dataHeadLength + m_dataTailLength + m_commIdentLength;
        int totalLength             = contentLength + Util::intWidth(contentLength) + Static::s_headerLen;
        
        char* buffer                = new char[totalLength + 1];
        memset(buffer, 0, totalLength + 1);
        
        sprintf(buffer, BaseStatic::s_streamTemplate, contentLength, m_dataHead, m_dataTail, m_commIdent);
        
        delete[] m_target;
        delete[] m_dataHead;
        delete[] m_dataTail;
        delete[] m_commIdent;
        
        m_rawOutput                 = buffer;
    } else {
        if (m_target) delete[] m_target;
        if (m_dataHead) delete[] m_dataHead;
        if (m_dataTail) delete[] m_dataTail;
        if (m_commIdent) delete[] m_commIdent;
    }
    
    m_write.start(m_fd, ev::WRITE);
}

void BaseRequest::dispatch() {
    if (*static_cast<char*>(m_path + 1) == '\0') {
        sendLanding();
    } else {
        sendStream(m_path + 1);
    }
}
