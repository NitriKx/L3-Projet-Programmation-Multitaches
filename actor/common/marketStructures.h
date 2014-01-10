//
//  marketStructures.h
//  actor
//
//  Created by Benoît Sauvère on 10/01/2014.
//  Copyright (c) 2014 Benoît Sauvère. All rights reserved.
//

#ifndef actor_marketStructures_h
#define actor_marketStructures_h

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



#endif
