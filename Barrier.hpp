#ifndef _BARRIER_H_
#define _BARRIER_H_

#include "Headers.hpp"

class Barrier {
    private:
    int counter;
    pthread_cond_t c;
    pthread_mutex_t m;

    public:
    Barrier() : counter(0) {
        pthread_mutex_init(&m, NULL);
        pthread_cond_init(&c, NULL);
    }

    void increase() {
        pthread_mutex_lock(&m);
        counter++;
        pthread_mutex_unlock(&m);
    }
    
    void decrease() {
        pthread_mutex_lock(&m);
        counter--;
        if (counter == 0) {
            pthread_cond_signal(&c);
        }
        pthread_mutex_unlock(&m);
    }

    void wait() {
        pthread_mutex_lock(&m);
        while (counter > 0) {
            pthread_cond_wait(&c, &m);
        }
        pthread_mutex_unlock(&m);
    }
};

#endif
