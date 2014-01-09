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
 Send an order in order to register this actor into the makretServer
 @param pipe_marketServer a file descriptor of the market server pipe
 **/
void sendRegisterOrder(int pipe_marketServer) {
    struct order registrationOrder;
    registrationOrder.sender = getpid();
    registrationOrder.type = OT_REGISTER;
    
    write(pipe_marketServer, &registrationOrder, sizeof(struct order));
}

/**
 Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param actionType
 @param maxQuantity
 @param maxPrice
 @param isBuy =1 buy, =0 sell
 @return the transaction report
 **/
struct transactionReport* sendTransactionOrder(int pipe_marketServer, int pipe_serverResponse, int actionType, int maxQuantity, int maxPrice, int isBuy) {
    
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
    
    // We block until we recieve th emarket server response
    read(pipe_serverResponse, responseReport, sizeof(transmitedOrder));
    
    return responseReport;
}


/**
 Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param actionType
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendBuyOrder(int pipe_marketServer, int pipe_serverResponse, int actionType, int maxQuantity, int maxPrice) {
    return sendTransactionOrder(pipe_marketServer, pipe_serverResponse, actionType, maxQuantity, maxPrice, 1);
}

/**
 Send a sell order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param actionType
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendSellOrder(int pipe_marketServer, int pipe_serverResponse, int actionType, int maxQuantity, int maxPrice) {
    return sendTransactionOrder(pipe_marketServer, pipe_serverResponse, actionType, maxQuantity, maxPrice, 0);
}