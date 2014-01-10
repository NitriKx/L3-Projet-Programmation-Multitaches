//
//  market_server.h
//  actor_user
//
//  Created by Benoît Sauvère on 06/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#ifndef actor_user_market_server_h
#define actor_user_market_server_h

#include "config.h"
#include "marketfunc.h"

struct alarm {
    int actionType;
    int price;
};

struct actorData {
    int pid;
    int money;
    int nbStocks[NB_TYPES_ACTIONS];
    int pipeDescriptor;
    struct alarm alarmHigh;
    struct alarm alarmLow;
};

struct transactionReport {
    int quantity;
    int totalCost;
};

/**
 Start the market server.
 @param _baseDirectory the directory which contains / will contains all the files needed by the market server
**/
void startMarketServer(char *_baseDirectory);

/**
 This method listen to new order (in the marketServer pipe) and call the good handler for each one
 **/
void listenForMarketOrders();

/**
 Register an alarm (high/low).
 @param order the order containing the alarm request information
 **/
void registerAlarmForActor (struct order *order);

/**
 For an actorID, check if the alarms needs to be send and sends it if needed.
 @param actorID the ID of the actor
 **/
void checkAlarmAndSendNotifications (int actorID);

/**
 Check if the alarms have to be sent to the actors.
 **/
void checkAlarmAndSendNotificationsForAllActor ();

/**
 Execute a buy order. The server will try to buy as many as possible actions considering the maximum price, and then transmit a transactionReport to the actor.
 @param order a pointer to the order read by the server containing the informations about transaction
 **/
void executeOrderAndSendResponseToTheActor (struct order *order);

/**
 Send a transaction report to an actor.
 @param report the transactionReport
 @param actorID the ID of the actor
 **/
void sendTransactionReportToActor(struct transactionReport *report, int actorID);
/**
 Register a new Actor.
 @param actorPID the process PID of the actor
 **/
void newActorRegistrationHandler(int actorPID);

/**
 Find the ID of the actor with a designated PID.
 @param actorPID the PID of the actor
 @return the ID of the actor designated by the PID
 **/
int findActorIDUsingActorPID(int actorPID);

/**
 Send all the prices to the actor designed by its ID (index in the tab)
 @param actorID the index of the actor in the actor array
 **/
void sendAllThePricesToActor(int actorID);

/**
 Send an action price to the designed actor
 @param actorID the index of the actor in the actor array
 @param actionType the action type
 **/
void sendPriceToActor(int actorID, int actionType);

/**
 This methos create and open the pipe_marketServer (named pipe).
 @return pipe descriptor
**/
int createAndOpenMarketOrderPipe();

/**
 This method will create the actor pipe if needed and will open it.
 @param actorPID the PID of the actor process
 @return a file descriptor on the opened pipe (only write)
 **/
int createIfNeededAndOpenActorPipe(int actorPID);

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

/**
 This method will delete all the file used by the marketServer.
**/
void cleanupFilesAndPipes();


#endif
