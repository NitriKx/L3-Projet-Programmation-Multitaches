//
//  pipeAndNameManipulation.c
//  actor
//
//  Created by Benoît Sauvère on 09/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#include "config.h"
#include "logger.h"
#include "pipeAndNameManipulation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

char *baseDirectory = CONFIG_BASE_DIRECTORY;

/**s
 This method opens the pipe_marketServer (named pipe).
 @return pipe descriptor
 **/
int openMarketOrderPipe() {
    
    _log("INFO", "Creating pipe_marketServer...");
    
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
    
    free(pipeMarketServerPath);
    
    return pipeDescriptor;
}

/**
 This method will create the actor pipe and will open it.
 @param actorPID the PID of the actor process
 @return a file descriptor on the opened pipe (only write)
 **/
int createAndOpenActorPipe(int actorPID) {
    
    char *actorPipeFullPath = getActorPipePath(actorPID);
    
    // Create the pipe
    int mkfifoExecutionResult;
    if ((mkfifoExecutionResult = mkfifo(actorPipeFullPath, 0666)) < 0) {
        
        if(mkfifo(actorPipeFullPath, 0666) == EEXIST) {
            printf("Communication pipe to actor=[%d] already exists ! Aborting...", actorPID);
            
        } else {
            perror("Error when creating a pipe to actor ");
            exit(EXIT_FAILURE);
        }
    }
    
    // Open it
    int actorPipeDescriptor = open(actorPipeFullPath, O_WRONLY);
    if(actorPipeDescriptor < 0) {
        perror("Can not open in write mode the actor pipe");
        exit(EXIT_FAILURE);
    }
    
    free(actorPipeFullPath);
    
    return actorPipeDescriptor;
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

