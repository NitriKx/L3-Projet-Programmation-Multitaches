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
#include <errno.h>

#define PIPE_MARKET_SERVER_NAME "pipe_marketServer"
#define PIPE_ACTOR_PREFIX "pipe_client"

// This pipe is used to receive all the actor requests (market order, registration, ...)
int pipe_marketServerDescriptor = 0;

int nbActorRegistered;
struct actorData actorRegisteredData[MAX_ACTOR];

// The boolean controls the listening loop
int hasToListen = 1;

char *baseDirectory = NULL;

/**
 Start the market server.
 @param _baseDirectory the directory which contains / will contains all the files needed by the market server
 **/
void startMarketServer(char *_baseDirectory) {
    
    _log("INFO", "Starting...");
    
    baseDirectory = _baseDirectory;
    
    // Create the pipe used to receive actor requests
    cleanupFilesAndPipes();
    pipe_marketServerDescriptor = createAndOpenMarketOrderPipe();
    
    // Initialize the marketfunc engine
    _log("INFO", "Initializing the marketfunc engine...");
    marketfunc_init(NB_TYPES_ACTIONS);
    
    listenForMarketOrders();
    
    _log("INFO", "Cleaning up...");
    
    marketfunc_terminate();
    cleanupFilesAndPipes();
}

/**
 This method listen to new order (in the marketServer pipe) and call the good handler for each one
 **/
void listenForMarketOrders() {
    
    struct order *readOrder = malloc(sizeof(struct order));
    
    // alarm(50000);
    
    // Listening loop, waiting for market orders
    while(hasToListen) {
        
        _log("INFO", "Waiting for incoming order...");
        
        // Read the next market order
        if(read(pipe_marketServerDescriptor, readOrder, sizeof(struct order)) < 0) {
            perror("Can not read from the pipe_marketServer ");
            exit(EXIT_FAILURE);
        }
        
        char orderDescriptionLoggingMessage[1024];
        sprintf(orderDescriptionLoggingMessage, "Order received : * Sender : %d * Type : %d * Val1 : %d * Val2 : %d", readOrder->sender, readOrder->type, readOrder->val1, readOrder->val2);
        _log("INFO", orderDescriptionLoggingMessage);
        
        switch(readOrder->type) {
            case OT_REGISTER:
                newActorRegistrationHandler(readOrder->sender);
                break;
            case OT_REQUEST_PRICE:
                sendPriceToActor(readOrder->sender, readOrder->val1);
                break;
            case OT_BUY:
            case OT_SELL:
                executeOrderAndSendResponseToTheActor(readOrder);
                break;
            case OT_ALARM_LOW:
            case OT_ALARM_HIGH:
                registerAlarmForActor(readOrder);
                break;
        }
        
    }
    
    free(readOrder);
}

/**
 Register an alarm (high/low).
 @param order the order containing the alarm request information
 **/
void registerAlarmForActor (struct order *order) {
    
    int actorID = findActorIDUsingActorPID(order->sender);
    
    if (order->type == OT_ALARM_HIGH) {
        actorRegisteredData[actorID].alarmHigh.price = order->val1;
        actorRegisteredData[actorID].alarmHigh.actionType = order->val2;
    } else if (order->type == OT_ALARM_LOW) {
        actorRegisteredData[actorID].alarmLow.price = order->val1;
        actorRegisteredData[actorID].alarmLow.actionType = order->val2;
    } else {
        _log("ERROR", "Invalid alarm type.");
        return;
    }
    
}

/**
 Check if the alarms have to be sent to the actors.
 **/
void checkAlarmAndSendNotificationsForAllActor () {
    int i;
    for (i=0; i < nbActorRegistered; i++) {
        checkAlarmAndSendNotifications(i);
    }
}

/**
 For an actorID, check if the alarms needs to be send and sends it if needed.
 @param actorID the ID of the actor
 **/
void checkAlarmAndSendNotifications (int actorID) {
    int actionTypeHigh = actorRegisteredData[actorID].alarmHigh.actionType;
    int actionTypeLow = actorRegisteredData[actorID].alarmLow.actionType;
    
    // Check the high alarm
    if (actionTypeHigh >= 0 && actionTypeHigh < NB_TYPES_ACTIONS) {
        int currentPriceHigh = get_price(actionTypeHigh);
        if (currentPriceHigh > actorRegisteredData[actorID].alarmHigh.price) {
            kill(actorRegisteredData[actorID].pid, SIGUSR1);
            char message[1024];
            sprintf(message, "Sent HIGH ALARM for actor=[%d] - value=[%d]", actorID, currentPriceHigh);
            _log("INFO", message);
        }
    }
    // Check the low alarm
    if (actionTypeLow >= 0 && actionTypeLow < NB_TYPES_ACTIONS) {
        int currentPriceLow = get_price(actorRegisteredData[actorID].alarmHigh.actionType);
        if (currentPriceLow < actorRegisteredData[actorID].alarmLow.price) {
            kill(actorRegisteredData[actorID].pid, SIGUSR2);
            char message[1024];
            sprintf(message, "Sent LOW ALARM for actor=[%d] - value=[%d]", actorID, currentPriceLow);
            _log("INFO", message);
        }
    }
}


//TODO: replace 0 with the good actionType
/**
 Execute a buy/sell order. The server will try to buy/sell as many as possible actions considering the maximum/minimum price, and then transmit a transactionReport to the actor.
 @param order a pointer to the order read by the server containing the informations about transaction
 **/
void executeOrderAndSendResponseToTheActor (struct order *order) {
    
    int actorID = findActorIDUsingActorPID(order->sender);
    struct transactionReport report;
    report.quantity = 0;
    report.totalCost = 0;
    
    if (order->type == OT_BUY) {
        int i;
        int buyPrice;
        char logMessage[1024];
        for(i = 0; i < order->val1; i++) {
            
            // Get the action price to check that if the actor has enought money
            int currentActionPrice = get_price(0);
            if(currentActionPrice >= actorRegisteredData[actorID].money) {
                // NO ENOUGHT CASH
                sprintf(logMessage, "Actor %d can not buy action=[%d] (current price=[%d] - actor cash=[%d])", actorID, 0, currentActionPrice, actorRegisteredData[actorID].money);
                _log("INFO", logMessage);
                continue;
            }
            
            // If the maximum price was good enought
            if((buyPrice = buy(0, order->val2)) > 0) {
                // OK
                report.quantity++;
                report.totalCost += buyPrice;
                actorRegisteredData[actorID].money -= buyPrice;
                actorRegisteredData[actorID].nbStocks[0]++;
                sprintf(logMessage, "Actor %d buy action=[%d] (current price=[%d] - actor cash after operation=[%d])", actorID, 0, currentActionPrice, actorRegisteredData[actorID].money);
                _log("INFO", logMessage);
                
            } else {
                // TOO EXPENSIVE
                
                sprintf(logMessage, "Actor %d can not buy action=[%d] (buy price=[%d] - maximum price=[%d])", actorID, 0, currentActionPrice, order->val2);
                _log("INFO", logMessage);
            }
        }
        
    } else if (order->type == OT_SELL) {
        int i;
        int sellPrice;
        char logMessage[1024];
        for(i = 0; i < order->val1; i++) {
            
            // Check if the user owns at least one action
            if(actorRegisteredData[actorID].nbStocks[0] > 0) {
                // If the minimum price was good enought
                if((sellPrice = sell(0, order->val2)) > 0) {
                    report.quantity++;
                    report.totalCost += sellPrice;
                    actorRegisteredData[actorID].nbStocks[0]--;
                } else {
                    // PRICE TOO LOW
                    sprintf(logMessage, "Actor %d can not sell action=[%d] (price=[%d] - minimum price=[%d])", actorID, 0, get_price(0), actorRegisteredData[actorID].nbStocks[0]);
                    _log("INFO", logMessage);
                }
                
            } else {
                // NO ENOUGHT ACTION
                sprintf(logMessage, "Actor %d can not sell action=[%d] (quantity owned=[%d])", actorID, 0, actorRegisteredData[actorID].nbStocks[0]);
                _log("INFO", logMessage);
            }
            
            
        }
        actorRegisteredData[actorID].money += report.totalCost;
        
    } else {
        _log("ERROR", "Unknown transaction type !");
        return;
    }
    
    sendTransactionReportToActor(&report, actorID);
    
    checkAlarmAndSendNotificationsForAllActor ();
}

/**
 Send a transaction report to an actor.
 @param report the transactionReport
 @param actorID the ID of the actor
 **/
void sendTransactionReportToActor(struct transactionReport *report, int actorID) {
    write(actorRegisteredData[actorID].pipeDescriptor, report, sizeof(struct transactionReport));
}

/**
 Find the ID of the actor with a designated PID.
 @param actorPID the PID of the actor
 @return the ID of the actor designated by the PID
 **/
int findActorIDUsingActorPID(int actorPID) {
    int actorID = -1;
    int i;
    for(i = 0; i < nbActorRegistered; i++) {
        if(actorRegisteredData[i].pid == actorPID) {
            actorID = i;
            break;
        }
    }
    return actorID;
}

/**
 Send all the prices to the actor designed by its ID (index in the tab)
 @param actorID the index of the actor in the actor array
 **/
void sendAllThePricesToActor(int actorID) {
    int i = 0;
    for(i = 0; i < NB_TYPES_ACTIONS; i++) {
        sendPriceToActor(actorID, i);
    }
}

/**
 Send an action price to the designed actor
 @param actorID the index of the actor in the actor array
 @param actionType the action type
 **/
void sendPriceToActor(int actorID, int actionType) {
    int actionPrice = get_price(actionType);
    if(write(actorRegisteredData[actorID].pipeDescriptor, &actionPrice, sizeof(int)) <= 0) {
        char logMessage[1024];
        sprintf(logMessage, "Can not send the price to actor=[%d] for action=[%d]. Reason=[%s].", actorID, actionPrice, strerror(errno));
        _log("ERROR", logMessage);
    }
}

/**
 Register a new Actor.
 @param actorPID the process PID of the actor
 **/
void newActorRegistrationHandler(int actorPID) {
    
    char logMessage[1024];
    sprintf(logMessage, "New actor registration : PID=[%d]", actorPID);
    _log("INFO", logMessage);
    
    // Check if the client limit is reached
    if(nbActorRegistered >= MAX_ACTOR) {
        _log("WARNING", "Registered clients limit reached");
        return;
    }
    
    struct actorData *actorDataStructureToFill = &actorRegisteredData[nbActorRegistered];
    actorDataStructureToFill->pid = actorPID;
    actorDataStructureToFill->money = ACTOR_INITIAL_MONEY;
    actorDataStructureToFill->pipeDescriptor = openActorPipe(actorPID);
    
    actorDataStructureToFill->alarmHigh.actionType = -1;
    actorDataStructureToFill->alarmLow.actionType = -1;
    
    _log("INFO", "Client registered. Sending prices...");
    
    // Send the price throught the response pipe
    sendAllThePricesToActor(nbActorRegistered);
    
    _log("INFO", "Prices sent");
    
    nbActorRegistered++;
}

/**
 This method will create the actor pipe if needed and will open it.
 @param actorPID the PID of the actor process
 @return a file descriptor on the opened pipe (only write)
 **/
int openActorPipe(int actorPID) {
    
    char *actorPipeFullPath = getActorPipePath(actorPID);
    
    char logMessage[1024];
    sprintf(logMessage, "Opening pipe [%s]...", actorPipeFullPath);
    _log("INFO", logMessage);
    
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
 This method will clean all the resources used by the marketServer.
 **/
void cleanupFilesAndPipes() {
    _log("INFO", "Cleaning up files...");
    
    close(pipe_marketServerDescriptor);
    char *pipeMarketServerPath = getFileinBaseDirectoryPath(PIPE_MARKET_SERVER_NAME);
    unlink(pipeMarketServerPath);
    free(pipeMarketServerPath);
    
    
    
    int i = 0;
    for (i = 0; i < nbActorRegistered; i++) {
        close(actorRegisteredData[i].pipeDescriptor);
    }
    
}

/**
 This methos create and open the pipe_marketServer (named pipe).
 @return pipe descriptor
 **/
int createAndOpenMarketOrderPipe() {
    
    _log("INFO", "Creating pipe_marketServer...");
    
    // Buid the pipe path
    char *pipeMarketServerPath = getFileinBaseDirectoryPath(PIPE_MARKET_SERVER_NAME);
    
    // Create the pipe
    if (mkfifo(pipeMarketServerPath, 0666) < 0) {
        perror("Error when creating the pipe_marketServer");
        exit(EXIT_FAILURE);
    }
    
    // Open it
    _log("INFO", "Opening the pipe_marketServer : waiting for client in write mode...");
    int pipeDescriptor = open(pipeMarketServerPath, O_RDONLY);
    if (pipeDescriptor < 0) {
        perror("Can not open the pipe_marketServer");
        exit(EXIT_FAILURE);
    }
    
    _log("INFO", "pipe_marketServer opened");
    
    free(pipeMarketServerPath);
    
    return pipeDescriptor;
}


/**
 This method builds the full path for a path relative in the baseDirectory.
 @param pathInBaseDirectory the path within the base directory
 @return the full path of the file designed by the relative path
 **/
char* getFileinBaseDirectoryPath(char *pathInBaseDirectory) {
    unsigned long baseDirectorySize = sizeof(char) * (strlen(baseDirectory));
    unsigned long pathInBaseDirectorySize = sizeof(char) * (strlen(pathInBaseDirectory));
    
    char *filePath = malloc(sizeof(char)*2 + baseDirectorySize + pathInBaseDirectorySize);
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
    return getFileinBaseDirectoryPath(actorPipeFullPath);
}

