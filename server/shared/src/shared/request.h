/*
 *  request.h
 *  360io
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _REQUEST_H_INCLUDED_
#define _REQUEST_H_INCLUDED_

#define INPUT_BUFFER_LEN    2048
#define OUTPUT_BUFFER_LEN   4096

#include <ev++.h>
#include <iostream>

#include "util.h"

#include <err.h>

#include <stdint.h>
#include <sys/socket.h>

#define CRLF                        0x00000a0d
#define CRLFCRLF                    0x0a0d0a0d

#define HTTP_GET                    0x20544547
#define HTTP_POST                   0x54534F50

#define HEADER_HOST                 0x74736F48
#define HEADER_CONT                 0x746E6F43
#define HEADER_COOK                 0x6B6F6F43

#define HEADER_CONT_LENG            0x676E654C
#define HEADER_CONT_TYPE            0x65707954

#define RESPONSE_404 "HTTP/1.1 200 OK\r\nContent-Length: 9\r\nConnection: close\r\nContent-Type: text/html\r\nServer: 360io/0.1\r\n\r\nNot Found"
#define RESPONSE_404_LEN strlen("HTTP/1.1 200 OK\r\nContent-Length: 9\r\nConnection: close\r\nContent-Type: text/html\r\nServer: 360io/0.1\r\n\r\nNot Found")

enum OutputType {
    OUTPUT_NONE,
    OUTPUT_STATIC,
    OUTPUT_RAW,
    OUTPUT_FORMATTED
};

enum RequestState {
    REQUEST_START,
    REQUEST_LOOKING_FOR_PATH_START,
    REQUEST_LOOKING_FOR_PATH_ENDING,
    REQUEST_LOOKING_FOR_ENDLINE,
    REQUEST_LOOKING_FOR_HEADER_NAME,
    REQUEST_LOOKING_FOR_HEADER_VALUE,
    REQUEST_WAITING_FOR_BODY
};

enum Method {
    METHOD_POST,
    METHOD_GET
};

template <typename RequestType>
class Request {
public:
    Request(int fd) : 
        m_fd(fd), 
        m_completed(false),
        m_state(REQUEST_START),

        m_input(new char[INPUT_BUFFER_LEN]),
        m_inputOffset(m_input),
        m_inputConsumed(0),

        m_path(0),
        m_host(0), 
        m_contentLength(0), 
        m_contentType(0), 
        m_contentBody(0),
        
        m_cookie(0),
    
        m_output(0),
        m_staticOutput(0),
        m_rawOutput(0),

        m_outputType(OUTPUT_NONE),
        m_outputWritten(0),
        m_outputLength(0),
        
        m_start(Util::getTime()) {
            memset(m_input, 0, INPUT_BUFFER_LEN);
    }
    
    void start() {
        m_read.set<Request<RequestType>, &Request<RequestType>::onRead>(this); 
        m_write.set<Request<RequestType>, &Request<RequestType>::onWrite>(this);
        m_read.start(m_fd, ev::READ);
    }
    
    ~Request() {  
        m_read.stop();
        m_write.stop();
        close(m_fd);
        
        delete[] m_input;
        
        if (m_output) {
            delete[] m_output;
        }
        
        if (m_rawOutput) {
            delete[] m_rawOutput;
        }
    }
    
private:
    void onRead(ev::io &w, int revents) {
        int bytesRead; 
        if (revents & EV_READ) {
            bytesRead = read(m_fd, m_inputOffset, INPUT_BUFFER_LEN - (m_inputOffset - m_input));
        } else {
            onDisconnect();
            return;
        }
        
        if (bytesRead <= 0) {
            onDisconnect();
            return;
        } else {
            m_inputOffset += bytesRead;
        }
        
        RequestState state = m_state;
        char* headerName;
        char* headerValue;
        char* p = m_input + m_inputConsumed;
        
        uint32_t bits;
        if (state != REQUEST_WAITING_FOR_BODY) {
            char cur;
            
            for (; p < m_inputOffset; ++p) {
                cur = *p;
                
                switch (state) {
                    case REQUEST_START:                
                        if (cur == ' ') {
                            switch (FIRST_FOUR_BYTES(m_input)) {
                                case HTTP_GET:
                                    m_method = METHOD_GET;
                                    break;
                                    
                                case HTTP_POST:
                                    m_method = METHOD_POST;
                                    break;
                            }
                            
                            state = REQUEST_LOOKING_FOR_PATH_START;
                        }
                        break;
                        
                    case REQUEST_LOOKING_FOR_PATH_START:
                        if (cur == '/') {
                            m_path = p;
                            state = REQUEST_LOOKING_FOR_PATH_ENDING;
                        }
                        break;
                        
                    case REQUEST_LOOKING_FOR_PATH_ENDING:
                        if (cur == ' ') {
                            *p = '\0';
                            state = REQUEST_LOOKING_FOR_ENDLINE;
                        }
                        break;
                        
                    case REQUEST_LOOKING_FOR_ENDLINE:
                        if (!((FIRST_FOUR_BYTES(p) & 0x0000FFFF) ^ CRLF)) {
                            headerName = ++(++p);
                            state = REQUEST_LOOKING_FOR_HEADER_NAME;
                        }
                        break;
                        
                    case REQUEST_LOOKING_FOR_HEADER_NAME:
                        if (cur == ':') {
                            *p = '\0';
                            headerValue = ++(++p);
                            state = REQUEST_LOOKING_FOR_HEADER_VALUE;
                        }
                        break;
                        
                    case REQUEST_LOOKING_FOR_HEADER_VALUE:
                        bits = FIRST_FOUR_BYTES(p);
                        
                        if (!((bits & 0x0000FFFF) ^ CRLF)) {
                            *p = '\0';
                            
                            switch (FIRST_FOUR_BYTES(headerName)) {
                                case HEADER_HOST: // checking for host
                                    m_host = headerValue;
                                    break;
                                    
                                case HEADER_CONT: // checking for content headers
                                    switch (FIRST_FOUR_BYTES(headerName + 8)) {
                                        case HEADER_CONT_LENG: // checking for content-length
                                            m_contentLength = atoi(headerValue);
                                            break;
                                            
                                        case HEADER_CONT_TYPE: // checking for content-type
                                            m_contentType = headerValue;
                                            break;
                                    }
                                    break;
                                    
                                case HEADER_COOK: // checking for cookie
                                    m_cookie = headerValue;
                                    break;  
                            }
                            
                            if (bits == CRLFCRLF) {
                                m_contentBody = p + 4;
                                p = m_inputOffset;
                                state = REQUEST_WAITING_FOR_BODY;
                            } else {
                                headerName = ++(++p);
                                state = REQUEST_LOOKING_FOR_HEADER_NAME;
                            }
                        }
                        break;
                        
                    case REQUEST_WAITING_FOR_BODY:
                        break;
                }
                
            }
        }
        
        if (state == REQUEST_WAITING_FOR_BODY && (m_inputOffset - m_contentBody >= m_contentLength)) {
            m_read.stop();
            
            if (m_host) {
                reinterpret_cast<RequestType*>(this)->dispatch();
            } else {
                m_write.start(m_fd, ev::WRITE);
            }
        } else {
            m_inputConsumed     = p - m_input;
            m_state             = state;
            
            m_headerName        = headerName;
            m_headerValue       = headerValue;
        }
    }

    void onWrite(ev::io &w, int revents) {
        if (revents & EV_WRITE) {
            if (!m_outputWritten) {
                if (m_staticOutput) {
                    m_outputLength          = strlen(m_staticOutput);
                    m_outputType            = OUTPUT_STATIC;
                } else if (m_rawOutput) {
                    m_outputLength          = strlen(m_rawOutput);
                    m_outputType            = OUTPUT_RAW;
                } else if (m_output) {
                    int bufferLen           = strlen(m_output);                    
                    char* buffer            = new char[bufferLen + 512];
                    
                    sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Length: %i\r\nConnection: close\r\nContent-Type: text/html\r\nServer: 360io/0.1\r\n\r\n%s", bufferLen, m_output);

                    m_outputLength          = strlen(buffer);
                    m_output                = buffer;
                    m_outputType            = OUTPUT_FORMATTED;
                } else {
                    m_outputLength          = RESPONSE_404_LEN;
                }
            }
            
            int written                     = m_outputWritten;
            int length                      = m_outputLength;
            int remaining                   = length - written;
            
            switch(m_outputType) {
                case OUTPUT_NONE:
                    written                 += write(m_fd, static_cast<const char*>(RESPONSE_404) + written, remaining);
                break;
                
                case OUTPUT_STATIC:
                    written                 += write(m_fd, m_staticOutput + written, remaining);
                break;
                
                case OUTPUT_RAW:
                    written                 += write(m_fd, m_rawOutput + written, remaining);
                break;
                
                case OUTPUT_FORMATTED:
                    written                 += write(m_fd, m_output + written, remaining);
                break;
            }
            
            if (written == length) {
                switch(m_outputType) {
                    case OUTPUT_NONE:
                    break;

                    case OUTPUT_STATIC:
                        m_staticOutput      = 0;
                    break;

                    case OUTPUT_RAW:
                        delete[] m_rawOutput;
                        m_rawOutput         = 0;
                    break;

                    case OUTPUT_FORMATTED:
                        delete[] m_output;
                        m_output            = 0;
                    break;
                }
                
                Util::logTime(m_host, m_path, m_start);

                m_write.stop();
                delete reinterpret_cast<RequestType*>(this);             
            } else {
                m_outputWritten             = written;
            }
        } else {
            onDisconnect();
        }
    }
    
    void onDisconnect() {
        RequestType* ptr = reinterpret_cast<RequestType*>(this);
        ptr->handleDisconnect();
        delete ptr;
    }
    
    
protected:
    int             m_fd;
    bool            m_completed;
    
    ev::io          m_read;
    ev::io          m_write;
    
    RequestState    m_state;
    Method          m_method;
    
    char*           m_input;
    char*           m_inputOffset;
    int             m_inputConsumed;
    
    char*           m_path;
    char*           m_host;
    
    int             m_contentLength;
    char*           m_contentType;
    char*           m_contentBody;
    
    char*           m_cookie;
    
    char*           m_headerName;
    char*           m_headerValue;
    
    char*           m_output;    
    char*           m_staticOutput;
    char*           m_rawOutput;
    
    OutputType      m_outputType;
    int             m_outputWritten;
    int             m_outputLength;
    
    TIMER_T         m_start;

};

#endif
