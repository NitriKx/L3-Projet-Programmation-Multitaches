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
int* sendRegisterOrder(int pipe_marketServer, int pipe_serverResponse) {
    struct order registrationOrder;
    registrationOrder.sender = getpid();
    registrationOrder.type = OT_REGISTER;
    
    // Send the registration
    write(pipe_marketServer, &registrationOrder, sizeof(struct order));
    
    // Wait for all the action prices
    int i = 0;
    int *actionPrices = malloc(sizeof(int) * NB_TYPES_ACTIONS);
    for (i = 0; i < NB_TYPES_ACTIONS; i++) {
        read(pipe_serverResponse, &actionPrices[i], sizeof(int));
    }
    
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
    
    struct transactionReport *responseReport = malloc(sizeof(struct transactionReport));
    
    // For now the action type is 1 beacause we are working on the 1st objective
    struct order transmitedOrder;
    transmitedOrder.sender = getpid();
    transmitedOrder.val1 = maxQuantity;
    transmitedOrder.val2 = maxPrice;
    
    if(isBuy > 0) {
        transmitedOrder.type = OT_BUY;
    } else {
        transmitedOrder.type = OT_SELL;
    }
    
    // Send the order to the market server
    write(pipe_marketServer, &transmitedOrder, sizeof(struct order));
    
    // We block until we recieve the market server response
    read(pipe_serverResponse, responseReport, sizeof(transmitedOrder));
    
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
