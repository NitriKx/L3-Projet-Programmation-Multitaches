//
//  logger.c
//  market_server
//
//  Created by Benoît Sauvère on 06/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void _log(const char* tag, const char* message) {
    time_t now;
    time(&now);
    printf("%s [%s]: %s\n", ctime(&now), tag, message);
}
