#ifndef __THREAD_H
#define __THREAD_H
#include "Headers.hpp"
#include "PCQueue.hpp"
#include "Barrier.hpp"

typedef enum job_type_t {PHASE1, PHASE2, POISON} JOB_TYPE;

class Thread {
public:
	Thread(uint thread_id) : m_thread_id(thread_id) {} 

	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	bool start();

	/** Will not return until the internal thread has exited. */
	void join();

	/** Returns the thread_id **/
	uint thread_id()
	{
		return m_thread_id;
	}

protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
	virtual void thread_workload() = 0;
	uint m_thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use
private:
	static void * entry_func(void * thread) {((Thread *)thread)->thread_workload(); return NULL; }
	pthread_t m_thread;
};

class Job {
	public:
	uint start;
	uint stop;
	vector<vector<int>>* curr;
	vector<vector<int>>* next;
	JOB_TYPE type;
	public:
	Job(JOB_TYPE type, uint start = 0, uint stop = 0, vector<vector<int>>* curr = NULL, vector<vector<int>>* next = NULL) 
		: start(start), stop(stop), curr(curr), next(next), type(type) {}
	int execute(); 
	~Job() = default;
};

class LifeThread : public Thread {
	PCQueue<Job*>* jobs;
	Barrier* b;
	vector<double>* hist;
	pthread_mutex_t* hist_mutex;
	public:
	LifeThread(uint id, PCQueue<Job*>* jobs, Barrier* b, vector<double>* hist, pthread_mutex_t* hist_mutex) 
		: Thread(id), jobs(jobs), b(b), hist(hist), hist_mutex(hist_mutex) {
	}
	
	void thread_workload() override;
};

#endif
