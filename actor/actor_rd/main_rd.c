//
//  main.c
//  actor_rd
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
#include <time.h>

int *currentPrices = NULL;
int pipe_marketOrder = -1;
int pipe_serverResponse = -1;

struct actorData values;

void initialize ();
void sendRandomBuyAndSellOrder ();
int getRandomValue (int low, int high);

int main (int argc, const char * argv[])
{

    initialize();
    int isAlive = 1;
    
    // Send random orders 
    while(isAlive) {
        sendRandomBuyAndSellOrder();
        sleep(ORDER_DELAY);
    }
    
}

/**
 Initialize all the resources
 **/
void initialize () {
    pipe_marketOrder = openMarketOrderPipe();
    createActorPipe(getpid());

    currentPrices = sendRegisterOrder(pipe_marketOrder, &pipe_serverResponse);
    
    values.money = ACTOR_INITIAL_MONEY;
    values.pipeDescriptor = pipe_serverResponse;
}

/**
 Send random order (buy/sell) to the market server
 **/
void sendRandomBuyAndSellOrder () {
    int isBuy = getRandomValue(0, 1);
    int quantity = getRandomValue(1, DEFAULT_ACTION_QTTY);
    int maxPrice = getRandomValue(1, DEFAULT_ACTION_PRICE+50);
    
    sendTransactionOrder(pipe_marketOrder, pipe_serverResponse, quantity, maxPrice, isBuy);
}

/**
 Get a random integer value, between low (included) and high (excluded).
 @param low the lowest value
 @param high the highest value
 @return a random integer between low and high
 **/
int getRandomValue (int low, int high) {
    srand((unsigned int) time(NULL));
    return rand()%(high-low) + low;
}