#pragma once
#include "threadsafe_queue.h"
#include <atomic>
#include <functional>
#include <thread>

class  join_threads {
    std::vector<std::thread>& threads_;
public:
    join_threads(std::vector<std::thread> &threads) : threads_(threads){
    }
    ~join_threads(){
        for(auto& th : threads_){
            th.join();
        }
    }
};

class thread_pool {
    std::atomic_bool done;
    threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;
    void worker_thread()
    {
        while(!done)
        {
            std::function<void()> task;
            if(work_queue.try_pop(task)){
                task();
            }
            else {
                std::this_thread::yield();
            }
        }
    }
public:
    thread_pool(size_t th_count) : done(false), joiner(threads)
    {
        unsigned const thread_count = th_count;
        try {
            for(unsigned i = 0; i < thread_count; ++i) {
                threads.push_back(std::thread(&thread_pool::worker_thread, this));
            }
        } catch(...) {
            done = true;
            throw;
        }
    }
    ~thread_pool() {
        done = true;
    }

    template<typename FunctionType>
    void submit(FunctionType f) {
        work_queue.push(std::function<void()>(f));
    }
};


#endif //ASYNC_THREAD_POOL_H
