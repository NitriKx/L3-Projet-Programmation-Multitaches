//
//  transmission.h
//  actor
//
//  Created by Benoît Sauvère on 09/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#ifndef actor_transmission_h
#define actor_transmission_h

struct transactionReport {
    int quantity;
    int totalCost;
};

/**
 Send an order in order to register this actor into the makretServer
 @param pipe_marketServer a file descriptor of the market server pipe
 **/
void sendRegisterOrder(int pipe_marketServer);

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
struct transactionReport* sendTransactionOrder(int pipe_marketServer, int pipe_serverResponse, int actionType, int maxQuantity, int maxPrice, int isBuy);

/**
 Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param actionType
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendBuyOrder(int pipe_marketServer, int pipe_serverResponse, int actionType, int maxQuantity, int maxPrice);

/**
 Send a sell order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param actionType
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendSellOrder(int pipe_marketServer, int pipe_serverResponse, int actionType, int maxQuantity, int maxPrice);

#endif
