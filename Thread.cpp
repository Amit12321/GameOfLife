#include "Thread.hpp"
#include "Game.hpp"

extern int calculate_phase1(vector<vector<int>>& field, vector<vector<int>>& next_field, uint start, uint stop);
extern int calculate_phase2(vector<vector<int>>& field, vector<vector<int>>& next_field, uint start, uint stop);

bool Thread::start() {
    if (pthread_create(&m_thread, NULL, entry_func, (void*)this) != 0) {
        return false;
    }
    return true;
}

void Thread::join() {
    pthread_join(m_thread, NULL);
}

void LifeThread::thread_workload() {
    bool active = true;
    while (active) {
        Job* job = jobs->pop();
        //Start Timer
        auto gen_start = std::chrono::system_clock::now();
        if (job->execute() == -1) {
            active = false;
        }
        delete job;
        auto gen_end = std::chrono::system_clock::now();
        //Stop timer
        b->decrease();

        if (active) { 
            pthread_mutex_lock(hist_mutex);
            hist->push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
            pthread_mutex_unlock(hist_mutex);
        }
    }
}

int Job::execute() {
    if (type == PHASE1) {
        return calculate_phase1(*curr, *next, start, stop);
    } else if (type == PHASE2) {
        return calculate_phase2(*curr, *next, start, stop);
    } else if (type == POISON) {
        return -1;
    }
    return -1;  // Should not reach here
}