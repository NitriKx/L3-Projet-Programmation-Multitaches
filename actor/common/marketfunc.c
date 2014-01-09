#include "marketfunc.h"
int pipe_lock[2];

struct action_type{
  int action_price;
  int nb_actions;
  int price_accel;
  int price_speed;
};

int nb_action_type = 0;
struct action_type * action_types = NULL;

void fluctuation(struct action_type *at, int operation){
  if (operation * at->price_accel <= 0)
    if ((random() % 3) ==0) 
      at->price_accel = at->price_accel + operation;

  at->price_speed = at->price_speed + at->price_accel;
  if (at->price_speed > 10) at->price_speed = 10;
  if (at->price_speed < -10) at->price_speed = -10;

  at->action_price = at->action_price + at->price_speed;
  if (at->action_price < 10) at->action_price = 10;

  if (at->action_price < 10) at->price_accel = 10;
  if (at->action_price < 100) at->price_accel++;
  if (at->action_price > 200) at->price_accel--;
  if (at->action_price > 400) at->price_accel = -10;
}

int marketfunc_init(int _nb_action_type){
  long i,res;
  char c='a';
  srandom(getpid());
  nb_action_type = _nb_action_type;
  action_types = (struct action_type *) malloc(nb_action_type * sizeof(struct action_type));
  if (action_types == NULL){
    perror("Memory allocation error");
    return -1;
  }

  for (i=0;i<nb_action_type;i++){
    action_types[i].action_price = DEFAULT_ACTION_PRICE;
    action_types[i].nb_actions = DEFAULT_ACTION_QTTY;
    action_types[i].price_accel = 0;
    action_types[i].price_speed = 0;
  }

  res = pipe(pipe_lock);
  if (res == -1) {
    perror("Error creating lock pipe");
    exit(1);
  }
  res = write(pipe_lock[1], &c, sizeof(char));
  if (res == -1) {
    perror("Error creating lock pipe");
    exit(1);
  }

  return 0;
}

int buy(int action_type, int maxprice){
  int res;
  struct action_type * at = &(action_types[action_type]);
  if (at->nb_actions > 0 && at->action_price <= maxprice) {
    res = at->action_price;
    at->nb_actions--;
    fluctuation(at,1);
  }
  else {
    res = 0;
  }
  return res;
}

int sell(int action_type, int minprice){
  int res;
  struct action_type * at = &(action_types[action_type]);
  if (at->action_price >= minprice) {
    res = at->action_price;
    at->nb_actions++;
    fluctuation(at, -1);
  }
  else {
    res = 0;
  }
  return res;
}

int get_price(int action_type){
  printf("function getprice\n");
  printf("valeur = %d \n", action_types[action_type].action_price);
  return action_types[action_type].action_price;
}

int get_action_quantity(int action_type){
  return action_types[action_type].nb_actions;
}

void marketfunc_terminate(){
  free(action_types);
}

void lock(){
  char c;
  long res = read(pipe_lock[0], &c, sizeof(char));
  if (res == -1) {
    perror("Error lock");
    exit(1);
  }
}

void unlock(){
  char c='a';
  long res = write(pipe_lock[1], &c, sizeof(char));
  if (res == -1) {
    perror("Error unlock");
    exit(1);
  }
}
