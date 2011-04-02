/*
 *  util.h
 *  360io
 *
 *  Created by Lewis Zimmerman on 07/08/09.
 *  Copyright 2009 Qorporation. All rights reserved.
 *
 */

#ifndef _UTIL_H_INCLUDED_
#define _UTIL_H_INCLUDED_

#define FIRST_FOUR_BYTES(chars) (*(reinterpret_cast<uint32_t*>(chars)))
#define FIRST_TWO_BYTES(chars) (*(reinterpret_cast<uint16_t*>(chars)))

#define TIMER_T double

class Util {
public:
    static int intWidth(int);
    static int adjustedLen(char*, int);
    static double getTime();
    static void logTime(char*, char*, double);
    
};

#endif
