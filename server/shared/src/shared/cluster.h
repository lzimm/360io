/*
 *  cluster.h
 *  360io
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _CLUSTER_H_INCLUDED_
#define _CLUSTER_H_INCLUDED_

#define COMMAND_BUFFER_LEN  4096

#include <ev++.h>
#include <iostream>

#include "util.h"

#include <err.h>

#include <stdint.h>
#include <sys/socket.h>

#define NOT_FOUND           "-1 NONE\r\n"
#define NOT_FOUND_LEN       9

#define CRLF_16             0x0a0d

enum CommandState {
    COMMAND_LOOKING_FOR_CMD_LENGTH,
    COMMAND_LOOKING_FOR_CMD_TYPE,
    COMMAND_LOOKING_FOR_CMD_PARAM,
    COMMAND_WAITING_FOR_CMD_BODY
};

template <typename ConnectionType>
class ClusterConnection {
public:
    ClusterConnection(int fd) : 
        m_fd(fd),
        m_state(COMMAND_LOOKING_FOR_CMD_LENGTH),
    
        m_buffer(new char[COMMAND_BUFFER_LEN]),
        m_bufferOffset(m_buffer),
        m_bufferConsumed(0),
    
        m_output(0),
        m_outputWritten(0),
        m_outputLength(0) {
            memset(m_buffer, 0, COMMAND_BUFFER_LEN);
    }
    
    void start() {
        m_read.set<ClusterConnection, &ClusterConnection::onRead>(this);
        m_write.set<ClusterConnection, &ClusterConnection::onWrite>(this);
        m_read.start(m_fd, ev::READ);
    }
    
    ~ClusterConnection() {
        m_read.stop();
        m_write.stop();
        close(m_fd);
        
        delete[] m_buffer;
        
        if (m_output) {
            delete[] m_output;
        }
    }
    
private:
    void onRead(ev::io& w, int revents) {
        int bytesRead;
        if (revents & EV_READ) {
            bytesRead = read(m_fd, m_bufferOffset, COMMAND_BUFFER_LEN - (m_bufferOffset - m_buffer));
        } else {
            onDisconnect();
            return;
        }
        
        if (bytesRead <= 0) {
            onDisconnect();
            return;
        } else {
            m_bufferOffset += bytesRead;
        }
        
        processBuffer();
    }
    
    void processBuffer() {
        CommandState state = m_state;
        char* p = m_buffer + m_bufferConsumed;
        
        if (state != COMMAND_WAITING_FOR_CMD_BODY) {
            char cur;
            
            for (; p < m_bufferOffset; ++p) {
                cur = *p;
                
                switch (state) {
                    case COMMAND_LOOKING_FOR_CMD_LENGTH:                
                        if (cur == ' ') {
                            *p = '\0';
                            m_commandLength = atoi(m_buffer);
                            m_command = ++p;
                            state = COMMAND_LOOKING_FOR_CMD_TYPE;
                        }
                        break;
                        
                    case COMMAND_LOOKING_FOR_CMD_TYPE:                
                        if (cur == ' ') {
                            *p = '\0';
                            m_commandParam = ++p;
                            state = COMMAND_LOOKING_FOR_CMD_PARAM;
                        }
                        break;
                        
                    case COMMAND_LOOKING_FOR_CMD_PARAM:                
                        if (cur == ' ' || FIRST_TWO_BYTES(p) == CRLF_16) {
                            *p = '\0';
                            m_commandBody = ++p;
                            p = m_bufferOffset;
                            state = COMMAND_WAITING_FOR_CMD_BODY;
                        }
                        break;
                        
                    case COMMAND_WAITING_FOR_CMD_BODY:
                        break;
                }
                
            }
        }
        
        int bytesAvailable  = m_bufferOffset - m_commandBody;
        if (state == COMMAND_WAITING_FOR_CMD_BODY && (bytesAvailable >= m_commandLength)) {
            m_read.stop();
            
            *(m_commandBody + m_commandLength) = '\0';
            reinterpret_cast<ConnectionType*>(this)->dispatch();
        } else {
            m_bufferConsumed = p - m_buffer;
            m_state = state;
            
            if (!m_read.active) {
                m_read.start(m_fd, ev::READ);
            }
        }
    }
    
    void onWrite(ev::io &w, int revents) {
        if (revents & EV_WRITE) {
            if (m_output) {
                int written                 = m_outputWritten;
                
                if (written == 0) {
                    int bufferLen           = strlen(m_output);
                    int lenWidth            = Util::intWidth(bufferLen);
                    int fullWidth           = bufferLen + lenWidth + 3;
                    char* buffer            = new char[fullWidth + 1];
                    *(buffer + fullWidth)   = '\0';
                
                    sprintf(buffer, "%i %s\r\n", bufferLen, m_output);
                    
                    delete[] m_output;
                    
                    m_outputLength          = fullWidth;
                    m_output                = buffer;
                }
                
                written                     += write(m_fd, m_output + written, m_outputLength - written);
                
                if (written == m_outputLength) {
                    delete[] m_output;
                    
                    m_outputWritten         = 0;
                    m_output                = 0;
                } else {
                    m_outputWritten         = written;
                }
            } else {
                int written                 = m_outputWritten;
                
                written                     += write(m_fd, NOT_FOUND + written, NOT_FOUND_LEN - written);
                
                if (written == NOT_FOUND_LEN) {
                    m_outputWritten         = 0;
                } else {
                    m_outputWritten         += written;
                }
            }
            
            m_write.stop();
            
            processRemainder();
        } else {
            onDisconnect();
        }
    }
    
    void processRemainder() {
        int bytesAvailable  = m_bufferOffset - m_commandBody;
        int bufferUsed      = m_bufferOffset - m_buffer;
        
        m_bufferConsumed    = 0;
        m_state             = COMMAND_LOOKING_FOR_CMD_LENGTH; 
        
        if (bytesAvailable > m_commandLength + 2) {
            int fullLength = m_commandBody - m_buffer + m_commandLength + 2;
            int nextBytes = bufferUsed - fullLength;
            
            memcpy(m_buffer, m_buffer + fullLength, nextBytes);
            memset(m_buffer + nextBytes, 0, COMMAND_BUFFER_LEN - nextBytes);
            
            m_bufferOffset = m_buffer + nextBytes;
            
            processBuffer();
        } else {
            m_bufferOffset = m_buffer;
            m_read.start(m_fd, ev::READ);
        }
    }
    
    void onDisconnect() {
        ConnectionType* ptr = reinterpret_cast<ConnectionType*>(this);
        ptr->handleDisconnect();
        delete ptr;
    }
    
protected:
    int             m_fd;
    
    ev::io          m_read;
    ev::io          m_write;
    
    CommandState    m_state;
        
    char*           m_buffer;
    char*           m_bufferOffset;
    int             m_bufferConsumed;

    int             m_commandLength;
    
    char*           m_command;
    char*           m_commandParam;
    char*           m_commandBody;
    
    char*           m_output;
    int             m_outputWritten;
    int             m_outputLength;
    
};

#endif
