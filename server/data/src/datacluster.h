/*
 *  datacluster.h
 *  360data
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _DATACLUSTER_H_INCLUDED_
#define _DATACLUSTER_H_INCLUDED_

#include "shared/cluster.h"

class DataClusterConnection : public ClusterConnection<DataClusterConnection> {
public:
    DataClusterConnection(int fd) : 
        ClusterConnection<DataClusterConnection>::ClusterConnection(fd) {
    }
    
public:
    void dispatch();

public:
    void handleDisconnect() {}
    
};

#endif
