#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "p3150258-pizza1.h"

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0) {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    } else if (td->tv_sec < 0 && td->tv_nsec > 0) {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

struct timespec startClock() {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    return start;
}

struct timespec stopClock(struct timespec start) {
    struct timespec finish, delta;
    clock_gettime(CLOCK_REALTIME, &finish);
    sub_timespec(start, finish, &delta);
    return delta;
}

unsigned getRandomNumber(unsigned * seed, unsigned min, unsigned max) {
    unsigned range = max - min + 1;
    return (rand_r(seed) % (range)) +min;
}

double average_delay = 0;
double max_delay = 0;
unsigned active_cooks = 0;
unsigned active_ovens = 0;
pthread_mutex_t printlock, avglock, maxlock, cooklock, ovenlock;
pthread_cond_t cond_cook, cond_bake;

void check(int x) {
    if (x == -1) {
        printf("pthread failed \n");
        exit(1);
    }
}

void * customer(void *arg) {
    Order * order = (Order*) arg;

    pthread_mutex_lock(&printlock);
    printf("Customer will arrive with ID: %d will arrive at %d for %d pizzas \n", order->id, order->arrivalTime, order->how_many_pizzas);
    pthread_mutex_unlock(&printlock);

    sleep(order->arrivalTime);

    // arrive
    struct timespec start = startClock();

    pthread_mutex_lock(&printlock);
    printf("Customer arrived with ID: %d for %d pizzas \n", order->id, order->how_many_pizzas);
    printf("waiting for a cook \n");
    pthread_mutex_unlock(&printlock);

    pthread_mutex_lock(&cooklock);
    while (active_cooks == 0) {
        pthread_cond_wait(&cond_cook, &cooklock);
    }
    active_cooks -= 1;
    pthread_mutex_unlock(&cooklock);

    // cook
    pthread_mutex_lock(&printlock);
    printf("*** Preparing pizza for %d \n", order->id);
    pthread_mutex_unlock(&printlock);

    sleep(order->how_many_pizzas * T_PREP);

    pthread_mutex_lock(&printlock);
    printf("waiting for a oven for %d \n", order->id);
    pthread_mutex_unlock(&printlock);

    pthread_mutex_lock(&ovenlock);
    while (active_cooks == 0) {
        pthread_cond_wait(&cond_bake, &ovenlock);
    }
    active_ovens -= 1;
    pthread_mutex_unlock(&ovenlock);

    // bake
    pthread_mutex_lock(&printlock);
    printf("*** Baking pizza for %d \n", order->id);
    pthread_mutex_unlock(&printlock);

    sleep(T_BAKE);

    // exit
    pthread_mutex_lock(&cooklock);
    active_cooks += 1;
    pthread_mutex_unlock(&cooklock);

    pthread_mutex_lock(&ovenlock);
    active_ovens += 1;
    pthread_mutex_unlock(&ovenlock);

    pthread_cond_broadcast(&cond_cook);
    pthread_cond_broadcast(&cond_bake);

    struct timespec delta = stopClock(start);

    double seconds = ((int) delta.tv_sec) + delta.tv_nsec / 1000000000.0;

    pthread_mutex_lock(&printlock);
    printf("Customer with ID: %d exited successfully, delta: %d.%.9ld, s=%lf\n", order->id, (int) delta.tv_sec, delta.tv_nsec, seconds);
    pthread_mutex_unlock(&printlock);

    pthread_mutex_lock(&maxlock);
    if (seconds > max_delay) {
        max_delay = seconds;
    }
    pthread_mutex_unlock(&maxlock);

    pthread_mutex_lock(&avglock);
    average_delay = average_delay + seconds;
    pthread_mutex_unlock(&avglock);

    free(order);
    pthread_exit(0);
    return 0;
}

int main(int argc, char** argv) {
    unsigned ncust = 0;
    unsigned seed = 0;
    unsigned i = 0;

    switch (argc) {
        case 3:
            seed = atoi(argv[1]);
            ncust = atoi(argv[2]);
            break;
    }

    if (ncust == 0 || seed == 0) {
        printf("Invalid value or missing value for ncust or seed \n");
        return 0;
    } else {
        srand(seed);
        active_cooks = N_COOK;
        active_ovens = N_OVEN;
    }

    unsigned arrival = T0;

    pthread_t * ids = malloc(sizeof (pthread_t) * ncust);

    check(pthread_mutex_init(&printlock, 0));
    check(pthread_mutex_init(&avglock, 0));
    check(pthread_mutex_init(&maxlock, 0));
    check(pthread_mutex_init(&cooklock, 0));
    check(pthread_mutex_init(&ovenlock, 0));
    check(pthread_cond_init(&cond_bake, NULL));
    check(pthread_cond_init(&cond_cook, NULL));

    for (i = 0; i < ncust; i++) {
        pthread_t temp;

        pthread_mutex_lock(&printlock);
        printf("Creating customer: %d, arrival time at: %d sec \n", i, arrival);
        pthread_mutex_unlock(&printlock);

        Order * order = malloc(sizeof (Order));
        order->id = i;
        order->arrivalTime = arrival;
        order->how_many_pizzas = getRandomNumber(&seed, N_ORDER_LOW, N_ORDER_HIGH);
        order->completionTime = 0;

        pthread_create(&temp, NULL, customer, order);

        ids[i] = temp;
        arrival = arrival + getRandomNumber(&seed, T_ORDER_LOW, T_ORDER_HIGH);
    }

    pthread_mutex_lock(&printlock);
    printf("waiting for all customers to be serviced \n");
    pthread_mutex_unlock(&printlock);

////int pthread_join pthread_t my_thread , void value_ptr

    for (i = 0; i < ncust; i++) {
        pthread_join(ids[i], NULL);
    }

    free(ids);

    printf("----------------------------------------------- \n");
    printf("  Average delay: %.5lf \n", average_delay / ncust);
    printf("  Maximum delay: %.5lf \n", max_delay);
    printf("----------------------------------------------- \n");

    check(pthread_mutex_destroy(&printlock));
    check(pthread_mutex_destroy(&avglock));
    check(pthread_mutex_destroy(&maxlock));
    check(pthread_mutex_destroy(&cooklock));
    check(pthread_mutex_destroy(&ovenlock));
    check(pthread_cond_destroy(&cond_bake));
    check(pthread_cond_destroy(&cond_cook));

    return 0;
}

