#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"

// Single Producer - Multiple Consumer queue
template <typename T>
class PCQueue
{

public:
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it. 
	// Assumes multiple consumers.
	PCQueue() {
		pthread_mutex_init(&mutex, NULL);
	}

	T pop() {
		avail_items.down();
		T item;
		pthread_mutex_lock(&mutex);
		item = items.front();
		items.pop();
		pthread_mutex_unlock(&mutex);
		return item;
	}
	// Assumes single producer 
	void push(const T& item) {
		pthread_mutex_lock(&mutex);
		items.push(item);
		pthread_mutex_unlock(&mutex);
		avail_items.up();
	}

private:
	queue<T> items;
	Semaphore avail_items;
	pthread_mutex_t mutex;
};


#endif