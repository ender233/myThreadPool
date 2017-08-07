#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <list>
#include <queue>
#include <functional>
#include <condition_variable>

using namespace std;
using Task = std::function<void()>;

class ThreadPool
{
    int                        _threadNums;
    vector<shared_ptr<thread>> _threads;
    condition_variable         _cv;
    std::mutex                 _m;

    bool                       _notEmpty;
    bool                       _running;
    queue<Task>                _tasks;
    int                        _timeout;

public:
    ThreadPool(int threadNums = std::thread::hardware_concurrency(), int timeout = -1) : 
        _threadNums(threadNums), 
        _running(true),
        _notEmpty(false),
        _timeout(timeout)
    {
        for(int i=0; i<_threadNums; ++i) {
            _threads.push_back(make_shared<std::thread>(&ThreadPool::runThread, this));
        }
    }

    ~ThreadPool() {stopAllTask();}

    void addTask(Task &&task)
    {
        std::unique_lock<mutex> lock(_m);
        _tasks.emplace(std::forward<Task>(task));
        _cv.notify_one();
        _notEmpty = true;
    }

    template<class F, class...Args>
    void addTask(F &&f, Args&&... args)
    {
        std::unique_lock<mutex> lock(_m);
        _tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        _cv.notify_one();
        _notEmpty = true;
    }

    void stopAllTask(void)
    {
        {
            std::unique_lock<mutex> lock(_m);
            _running = false;    
            _cv.notify_all();
        }

        for(auto &t : _threads) {
            t->join(); 
        }
        _threads.clear();
    }

private:
    void runThread() {
        while(_running) {
            Task task;
            {
                std::unique_lock<mutex> lock(_m);
                _cv.wait(lock, [this](){return (_notEmpty) || (!_running);});
                if(!_running) {
                    return;
                }

                task = _tasks.front();
                _tasks.pop();
                if(_tasks.empty()) _notEmpty=false;
            }

            // finish thread after timeout
            if(_timeout == -1) {
                task(); 
                continue;
            }
            do {
                condition_variable cv;
                std::thread t([&](){
                       task(); 
                       cv.notify_one();
                    });
                t.detach();

                std::mutex m1;
                std::unique_lock<mutex> lock(m1);
                if(cv.wait_for(lock, _timeout*1s) == std::cv_status::timeout) {
                    std::cout<<"time out, cancel the thread"<<std::endl; 
                    break;
                }
            } while(0);
        }
    }
};
