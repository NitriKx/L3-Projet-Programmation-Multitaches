//
//  market_server.c
//  actor_user
//
//  Created by Benoît Sauvère on 06/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#include "config.h"
#include "market_server.h"
#include "marketfunc.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define PIPE_MARKET_SERVER_NAME "pipe_marketServer"
#define PIPE_ACTOR_PREFIX "pipe_client"

// This pipe is used to receive all the actor requests (market order, registration, ...)
int pipe_marketServerDescriptor = 0;

int nbActorRegistered;
struct actorData actorRegisteredData[MAX_ACTOR];

char *baseDirectory = NULL;

/**
 Start the market server.
 @param _baseDirectory the directory which contains / will contains all the files needed by the market server
 **/
void startMarketServer(char *_baseDirectory) {
    
    log("INFO", "Starting...");
    
    baseDirectory = _baseDirectory;
    
    // Create the pipe used to receive actor requests
    cleanupFiles();
    pipe_marketServerDescriptor = createAndOpenMarketOrderPipe();
    
    // Initialize the marketfunc engine
    log("INFO", "Initializing the marketfunc engine...");
    marketfunc_init(NB_TYPES_ACTIONS);
    
    
}


/**
 Register a new Actor.
 @param actorPID the process PID of the actor
 **/
void newActorRegistrationHandler(int actorPID) {
    
    // Check if the client limit is reached
    if(nbActorRegistered >= MAX_ACTOR) {
        log("WARNING", "Registered clients limit reached");
        return;
    }
    
    struct actorData *actorDataStructureToFill = actorRegisteredData + nbActorRegistered;
    actorDataStructureToFill->pid = actorPID;
    actorDataStructureToFill->money = actorPID;
    actorDataStructureToFill->pid = actorPID;
    actorDataStructureToFill->pid = actorPID;
    
    nbActorRegistered++;
}

/**
 This method will create the actor pipe if needed and will open it.
 @param actorPID the PID of the actor process
 @return a file descriptor on the opened pipe (only write)
 **/
int createIfNeededAndOpenActorPipe(int actorPID) {
    
    char *actorPipeFullPath = getActorPipePath(actorPID);
    
    int actorPipeDescriptor = open(actorPipeFullPath, O_WRONLY);
    if(actorPipeDescriptor < 0) {
        perror("Can not open in write mode the actor pipe");
        exit(EXIT_FAILURE);
    }
    
    free(actorPipeFullPath);
    
    return actorPipeDescriptor;
}

/**
 This methos create and open the pipe_marketServer (named pipe).
 @return pipe descriptor
 **/
int createAndOpenMarketOrderPipe() {
    
    log("INFO", "Creating pipe_marketServer...");
    
    // Buid the pipe path
    char *pipeMarketServerPath = getFileinBaseDirectoryPath(PIPE_MARKET_SERVER_NAME);
    
    // Create the pipe
    if (mkfifo(pipeMarketServerPath, 0666) < 0) {
        perror("Error when creating the pipe_marketServer");
        exit(EXIT_FAILURE);
    }
    
    // Open it
    int pipeDescriptor = open(pipeMarketServerPath, O_RDONLY);
    if (pipeDescriptor <= 0) {
        perror("Can not open the pipe_marketServer");
        exit(EXIT_FAILURE);
    }
    
    return pipeDescriptor;
}


/**
 This method will clean all the resources used by the marketServer.
 **/
void cleanupFiles() {
    log("INFO", "Cleaning up files...");
    close(pipe_marketServerDescriptor);
    char *pipeMarketServerPath = getFileinBaseDirectoryPath(PIPE_MARKET_SERVER_NAME);
    unlink(pipeMarketServerPath);
    free(pipeMarketServerPath);
}


/**
 This method builds the full path for a path relative in the baseDirectory.
 @param pathInBaseDirectory the path within the base directory
 @return the full path of the file designed by the relative path
 **/
char* getFileinBaseDirectoryPath(char *pathInBaseDirectory) {
    unsigned long baseDirectorySize = sizeof(char) * (strlen(baseDirectory));
    unsigned long pathInBaseDirectorySize = sizeof(char) * (strlen(pathInBaseDirectory));
    
    char *filePath = malloc(sizeof(char) + baseDirectorySize + pathInBaseDirectorySize);
    memcpy(filePath, baseDirectory, baseDirectorySize);
    filePath[baseDirectorySize] = '/';
    memcpy(filePath+baseDirectorySize+1, pathInBaseDirectory, pathInBaseDirectorySize+1);
    
    return filePath;
}

/**
 @param actorPID the PID of the actor you want
 @return the full path for the
 **/
char* getActorPipePath(int actorPID) {
    int pidMaxSize = 5 * sizeof(char);
    char *pipeActorPrefix = PIPE_ACTOR_PREFIX;
    char *actorPipeFullPath = malloc((pidMaxSize + strlen(pipeActorPrefix) + 1) * sizeof(char));
    sprintf(actorPipeFullPath, "%s%d", pipeActorPrefix, actorPID);
    return actorPipeFullPath;
}

