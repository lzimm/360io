/*
 *  commcluster.h
 *  360comm
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _COMMCLUSTER_H_INCLUDED_
#define _COMMCLUSTER_H_INCLUDED_

#include "shared/cluster.h"

class CommClusterConnection : public ClusterConnection<CommClusterConnection> {
public:
    CommClusterConnection(int fd) :
        ClusterConnection<CommClusterConnection>::ClusterConnection(fd) {
    }
    
public:
    void dispatch();
    
public:
    void handleDisconnect() {}
    
};

#endif
