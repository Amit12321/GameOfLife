#include "Semaphore.hpp"

Semaphore::Semaphore(unsigned counter) : counter(counter) {
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&avail_ticket, NULL);
}

void Semaphore::up() {
    pthread_mutex_lock(&m);
    counter++;
    pthread_cond_signal(&avail_ticket);
    pthread_mutex_unlock(&m);
}

void Semaphore::down() {
    pthread_mutex_lock(&m);
    while (counter == 0) {
        pthread_cond_wait(&avail_ticket, &m);
    }
    counter--;
    pthread_mutex_unlock(&m);
}

