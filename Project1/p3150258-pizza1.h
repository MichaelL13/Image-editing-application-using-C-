
#ifndef HEADER_H
#define HEADER_H

typedef struct Order {
    unsigned id;
    unsigned arrivalTime;
    unsigned completionTime;
    int how_many_pizzas;
} Order;

enum {
    NS_PER_SECOND = 1000000000
};
  
#define N_COOK 6
#define N_OVEN 5
#define T_ORDER_LOW 1
#define T_ORDER_HIGH 5
#define N_ORDER_LOW 1
#define N_ORDER_HIGH 5
#define T_PREP 1
#define T_BAKE 10

#define T0 0


// https://stackoverflow.com/questions/53708076/what-is-the-proper-way-to-use-clock-gettime
//
//struct timespec start = startClock();
//    sleep(1);
//    struct timespec deleta = stopClock(start);
//  

#endif /* HEADER_H */

