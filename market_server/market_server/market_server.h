//
//  market_server.h
//  actor_user
//
//  Created by Benoît Sauvère on 06/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#ifndef actor_user_market_server_h
#define actor_user_market_server_h

#include "config.h"

struct actorData {
    int pid;
    int money;
    int nbStocks[NB_TYPES_ACTIONS];
    int pipeDescriptor;
};

/**
 Start the market server.
 @param _baseDirectory the directory which contains / will contains all the files needed by the market server
**/
void startMarketServer(char *_baseDirectory);

/**
 This methos create and open the pipe_marketServer (named pipe).
 @return pipe descriptor
**/
int createAndOpenMarketOrderPipe();

/**
 This method builds the full path for a path relative in the baseDirectory.
 @param pathInBaseDirectory the path within the base directory
 @return the full path of the file designed by the relative path
 **/
char* getFileinBaseDirectoryPath(char *pathInBaseDirectory);

/**
 @param actorPID the PID of the actor you want
 @return the full path for the
 **/
char* getActorPipePath(int actorPID);

/**
 This method will delete all the file used by the marketServer.
**/
void cleanupFiles();


#endif
