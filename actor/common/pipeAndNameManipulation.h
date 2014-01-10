//
//  pipeAndNameManipulation.h
//  actor
//
//  Created by Benoît Sauvère on 09/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#ifndef actor_pipeAndNameManipulation_h
#define actor_pipeAndNameManipulation_h

#define PIPE_MARKET_SERVER_NAME "pipe_marketServer"
#define PIPE_ACTOR_PREFIX "pipe_client"

/**s
 This method opens the pipe_marketServer (named pipe).
 @return pipe descriptor
 **/
int openMarketOrderPipe();

/**
 This method will create the actor pipe.
 @param actorPID the PID of the actor process
 **/
void createActorPipe(int actorPID);

/**
 Blocking - Opens the actor pipe associated with this actor PID
 @param actorPID the PID of the actor
 @return a file descriptor pointing to the pipe
 **/
int openActorPipe(int actorPID);

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

#endif
