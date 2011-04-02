/*
 *  synccluster.h
 *  360sync
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _SYNCCLUSTER_H_INCLUDED_
#define _SYNCCLUSTER_H_INCLUDED_

#include "shared/cluster.h"

class SyncClusterConnection : public ClusterConnection<SyncClusterConnection> {
public:
    SyncClusterConnection(int fd) : 
        ClusterConnection<SyncClusterConnection>::ClusterConnection(fd) {
    }
    
public:
    void dispatch();

public:
    void handleDisconnect() {}
    
};

#endif
