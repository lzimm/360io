/*
 *  logcluster.h
 *  360log
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _LOGCLUSTER_H_INCLUDED_
#define _LOGCLUSTER_H_INCLUDED_

#include "shared/cluster.h"

class LogClusterConnection : public ClusterConnection<LogClusterConnection> {
public:
    LogClusterConnection(int fd) : 
        ClusterConnection<LogClusterConnection>::ClusterConnection(fd) {
    }
    
public:
    void dispatch();

public:
    void handleDisconnect() {}
    
};

#endif
