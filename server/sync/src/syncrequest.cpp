/*
 *  syncrequest.cpp
 *  360sync
 *
 *  Created by Lewis Zimmerman on 25/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "syncrequest.h"
#include "manager.h"

#define HEADER_HOST                 0x74736F48
#define HEADER_CONT                 0x746E6F43
#define HEADER_COOK                 0x6B6F6F43

#define HEADER_CONT_LENG            0x676E654C
#define HEADER_CONT_TYPE            0x65707954

#define DISPATCH_CONT               0x544E4F43

void SyncRequest::dispatch() {
    switch (FIRST_FOUR_BYTES(m_path + 1)) {
        case DISPATCH_CONT:
            m_output = static_cast<char*>(Manager::store.getCount(m_path + 6));
        break;
    }
    
    m_write.start(m_fd, ev::WRITE);
}
