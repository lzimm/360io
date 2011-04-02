/*
 *  manager.cpp
 *  360comm
 *
 *  Created by Lewis Zimmerman on 26/07/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#include "manager.h"

Dispatcher Manager::dispatcher;

void Manager::init() {
    dispatcher = Dispatcher();
}
