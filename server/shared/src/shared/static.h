/*
 *  static.h
 *  360io
 *
 *  Created by Lewis Zimmerman on 04/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _STATIC_H_INCLUDED_
#define _STATIC_H_INCLUDED_

#define HTTP_HEADERS "HTTP/1.1 200 OK\r\nContent-Length: %i\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nServer: 360io/0.1\r\n\r\n"

class Static {
protected:
    static char* read(char*);
    
public:
    static int s_headerLen;

};

#endif
