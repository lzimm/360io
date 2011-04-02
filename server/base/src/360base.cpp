/*
 *  360data.cpp
 *  360data
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <ev++.h>

#include "baserequest.h"
#include "manager.h"
#include "basestatic.h"

#include "shared/config.h"

static int setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    
    if (flags < 0) return flags;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    
    return 0;
}

void httpAccept(ev::io &w, int revents) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    memset(&client_addr, 0, client_len);
	int client_fd = accept(w.fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    
    if (client_fd == -1) return;
	if (setnonblock(client_fd) < 0) err(1, "failed to set client socket to non-blocking");
    
    BaseRequest* req = new BaseRequest(client_fd);
    req->start();
}

void setupHTTP(ev::io& http) {
    int reuseaddr_on                = 1;
    int http_fd                     = socket(AF_INET, SOCK_STREAM, 0); 
    
    if (http_fd < 0) err(1, "listen failed");
    if (setsockopt(http_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1) err(1, "setsockopt failed");
    
    sockaddr_in http_addr;
    memset(&http_addr, 0, sizeof(http_addr));
    http_addr.sin_family            = AF_INET;
    http_addr.sin_addr.s_addr       = INADDR_ANY;
    http_addr.sin_port              = htons(BASE_HTTP_PORT);
    
    if (bind(http_fd, reinterpret_cast<sockaddr*>(&http_addr), sizeof(http_addr)) < 0) err(1, "bind failed");
    if (listen(http_fd, 5) < 0) err(1, "listen failed");
    if (setnonblock(http_fd) < 0) err(1, "failed to set server socket to non-blocking");
    
    http.set<httpAccept>();
	http.start(http_fd, ev::READ);
}

int main() {
    ev::io http;
    
    setupHTTP(http);
    
    Manager::init();
    BaseStatic::init();
    ev::loop_ref loop               = EV_DEFAULT;
    loop.loop(EVBACKEND_EPOLL);
    
	return 0;
}
