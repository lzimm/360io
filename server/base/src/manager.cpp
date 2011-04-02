/*
 *  manager.cpp
 *  360base
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "manager.h"
#include "shared/config.h"

ConnPool Manager::m_dataPool(DATA_NODE_HOST, DATA_NODE_PORT);
ConnPool Manager::m_commPool(COMM_NODE_HOST, COMM_NODE_PORT);

void Manager::init() {
}

Conn* Manager::acquireDataConnection() {
    return m_dataPool.acquireConnection();
}

Conn* Manager::acquireCommConnection() {
    return m_commPool.acquireConnection();
}
