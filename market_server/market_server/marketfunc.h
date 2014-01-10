#ifndef __MARKETFUNC_H
#define __MARKETFUNC_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define OT_REGISTER 1
#define OT_ALARM_HIGH 2
#define OT_ALARM_LOW 3
#define OT_BUY 4
#define OT_SELL 5
#define OT_REQUEST_PRICE 6
#define DEFAULT_ACTION_PRICE 100
#define DEFAULT_ACTION_QTTY 50

struct order{
    int type;
    int sender;
    int val1;
    int val2;
};


int marketfunc_init(int _nb_action_types);

int buy(int action_type, int maxprice);

int sell(int action_type, int minprice);

int get_price(int action_type);

int get_action_quantity(int action_type);

void marketfunc_terminate();

void lock();

void unlock();
#endif
