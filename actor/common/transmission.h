//
//  transmission.h
//  actor
//
//  Created by Benoît Sauvère on 09/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#ifndef actor_transmission_h
#define actor_transmission_h

/**
 Blocking - Register this actor on the makretServer
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse  a file descriptor of the server response pipe
 **/
int* sendRegisterOrder(int pipe_marketServer, int *pipe_serverResponse);

/**
 Blocking - Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param maxQuantity
 @param maxPrice
 @param isBuy =1 buy, =0 sell
 @return the transaction report
 **/
struct transactionReport* sendTransactionOrder(int pipe_marketServer, int pipe_serverResponse, int maxQuantity, int maxPrice, int isBuy);

/**
 Blocking - Send a buy order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendBuyOrder(int pipe_marketServer, int pipe_serverResponse, int maxQuantity, int maxPrice);

/**
 Blocking - Send a sell order to the market server.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param pipe_serverResponse a file descriptor of the pipe used to read the server response
 @param maxQuantity
 @param maxPrice
 @return the transaction report
 **/
struct transactionReport* sendSellOrder(int pipe_marketServer, int pipe_serverResponse, int maxQuantity, int maxPrice);


/**
 Send an alarm registration.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param price the price limit actor wants to be notified (low or high)
 @param isHighAlarm =1 High, =0 Low
 **/
void registerAlarm(int pipe_marketServer, int price, int isHighAlarm);

/**
 Send an high alarm registration.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param price the price limit actor wants to be notified (high)
 **/
void registerHighAlarm(int pipe_marketServer, int price);

/**
 Send an high alarm registration.
 @param pipe_marketServer a file descriptor of the market server pipe
 @param price the price limit actor wants to be notified (low)
 **/
void registerLowAlarm(int pipe_marketServer, int price);

#endif
