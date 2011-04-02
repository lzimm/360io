/*
 *  synccluster.cpp
 *  360sync
 *
 *  Created by Lewis Zimmerman on 28/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "synccluster.h"
#include "manager.h"

#include "shared/util.h"

#define DISPATCH_CURR   0x52525543
#define DISPATCH_NEXT   0x5458454E

#define DISPATCH_CONT   0x544E4F43

void SyncClusterConnection::dispatch() {
    switch (FIRST_FOUR_BYTES(m_command)) {
        case DISPATCH_CURR:
            *(m_commandBody + m_commandLength) = '\0';
            
            switch (FIRST_FOUR_BYTES(m_command + 5)) {
                case DISPATCH_CONT:
                    m_output = static_cast<char*>(Manager::store.getCount(m_commandParam));
                break;
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
            
        case DISPATCH_NEXT:
            switch (FIRST_FOUR_BYTES(m_command + 5)) {
                case DISPATCH_CONT:
                    char* currBuff = static_cast<char*>(Manager::store.getCount(m_commandParam));
                    int curr = 0;

                    if (currBuff) {        
                        curr = atoi(currBuff);
                        delete[] currBuff;
                    }
                    
                    int len = Util::intWidth(++curr);
                    m_output = new char[len + 1];
                    sprintf(m_output, "%i", curr);
                    
                    Manager::store.putCount(len + 1, m_commandParam, m_output);
                break;
            }
            
            m_write.start(m_fd, ev::WRITE);
        break;
            
        default:
            m_read.start(m_fd, ev::READ);
        break;
    }
}
