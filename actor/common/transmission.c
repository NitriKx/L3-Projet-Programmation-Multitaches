//
//  transmission.c
//  actor
//
//  Created by Benoît Sauvère on 09/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#include "config.h"
#include "logger.h"
#include "pipeAndNameManipulation.h"
#include "transmission.h"
#include "marketfunc.h"
#include "marketStructures.h"
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


/**
 Blocking - Register this actor on the makretServer
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse  a file descriptor of the server response pipe
 **/
int* sendRegisterOrder(int pipe_marketServer, int *pipe_serverResponse) {

    _log("INFO", "Sending register order...");
    
    struct order registrationOrder;
    registrationOrder.sender = getpid();
    registrationOrder.type = OT_REGISTER;
    
    // Send the registration
    write(pipe_marketServer, &registrationOrder, sizeof(struct order));
    
    _log("INFO", "Register order sent. Opening the response tube...");
    
    // If the pipe is not already opened
    if(*pipe_serverResponse < 0) {
        *pipe_serverResponse = openActorPipe(getpid());
    }
    
    _log("INFO", "Response pipe opened. Fetching prices...");
    
    // Wait for all the action prices
    int i = 0;
    int *actionPrices = malloc(sizeof(int) * NB_TYPES_ACTIONS);
    for (i = 0; i < NB_TYPES_ACTIONS; i++) {
        if(read(*pipe_serverResponse, &(actionPrices[i]), sizeof(int))) {
            _log("WARNING", "Can not fetch the prices.");
            perror("Can not fetch the prices");
            return NULL;
        }
        
        char logMessage[1024];
        sprintf(logMessage, "Fetched price for action=[%d] price=[%d]", i, actionPrices[i]);
        _log("INFO", logMessage);
    }
    
    _log("INFO", "Prices fetched. Registration complete.");
    
    return actionPrices;
}


/**
 Blocking - Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param maxQuantity
 @param maxPrice
 @param isBuy =1 buy, =0 sell
 @return the transaction report
 **/
struct transactionReport* sendTransactionOrder(int pipe_marketServer, int pipe_serverResponse, int maxQuantity, int maxPrice, int isBuy) {
    
    _log("INFO", "Sending transaction...");
    
    struct transactionReport *responseReport = malloc(sizeof(struct transactionReport));
    
    // For now the action type is 1 beacause we are working on the 1st objective
    struct order transmitedOrder;
    transmitedOrder.sender = getpid();
    transmitedOrder.val1 = maxQuantity;
    transmitedOrder.val2 = maxPrice;
    
    char logMessage[1024];
    sprintf(logMessage, "Values :\nSender : %d\nisBuy : %d\nVal1 : %d\nVal2 : %d", transmitedOrder.sender, isBuy, transmitedOrder.val1, transmitedOrder.val2);
    _log("INFO", logMessage);
    
    if(isBuy > 0) {
        transmitedOrder.type = OT_BUY;
    } else {
        transmitedOrder.type = OT_SELL;
    }
    
    _log("INFO", "Writing transaction...");
    
    // Send the order to the market server
    write(pipe_marketServer, &transmitedOrder, sizeof(struct order));
    
    _log("INFO", "Waiting for response...");
    
    // We block until we recieve the market server response
    read(pipe_serverResponse, responseReport, sizeof(struct order));
    
    sprintf(logMessage, "Server response : \nQuantity : %d / Total Cost %d", responseReport->quantity, responseReport->totalCost);
    _log("INFO", logMessage);

    return responseReport;
}


/**
 Blocking - Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendBuyOrder(int pipe_marketServer, int pipe_serverResponse, int maxQuantity, int maxPrice) {
    return sendTransactionOrder(pipe_marketServer, pipe_serverResponse, maxQuantity, maxPrice, 1);
}

/**
 Blocking - Send a sell order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendSellOrder(int pipe_marketServer, int pipe_serverResponse, int maxQuantity, int maxPrice) {
    return sendTransactionOrder(pipe_marketServer, pipe_serverResponse, maxQuantity, maxPrice, 0);
}

/**
 Send an alarm registration.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param price the price limit actor wants to be notified (low or high)
 @param isHighAlarm =1 High, =0 Low
 **/
void registerAlarm(int pipe_marketServer, int price, int isHighAlarm) {
    
    struct order sendOrder;
    sendOrder.sender = getpid();
    sendOrder.val1 = price;
    
    if(isHighAlarm > 0) {
        sendOrder.type = OT_ALARM_HIGH;
    } else {
        sendOrder.type = OT_ALARM_LOW;
    }
    
    write(pipe_marketServer, &sendOrder, sizeof(struct order));
}

/**
 Send an high alarm registration.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param price the price limit actor wants to be notified (high)
 **/
void registerHighAlarm(int pipe_marketServer, int price) {
    registerAlarm(pipe_marketServer, price, 1);
}

/**
 Send an high alarm registration.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param price the price limit actor wants to be notified (low)
 **/
void registerLowAlarm(int pipe_marketServer, int price) {
    registerAlarm(pipe_marketServer, price, 0);
}
