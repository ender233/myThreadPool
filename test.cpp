#include <iostream>
#include <chrono>
#include "threadPool.hpp"

using namespace std;
using namespace std::chrono_literals;

void printXY(int x, int y) {
    std::cout<<"x:"<<x<<" y"<<y<<endl;
}

int main()
{
    // ThreadPool(int threadNums, int timeout)
    // timeout : * seconds
    ThreadPool pool(3, 2);
    pool.addTask([](){
               std::cout<<"task 1 current thread id is:"<<std::this_thread::get_id()<<std::endl;
            });
    pool.addTask([](){
               std::cout<<"task 2 current thread id is:"<<std::this_thread::get_id()<<std::endl;
            });
    pool.addTask([](){
               this_thread::sleep_for(3s);
               std::cout<<"task 3 current thread id is:"<<std::this_thread::get_id()<<std::endl;
            });
    pool.addTask(printXY, 2, 3);
    //pool.stopAllTask();
 
    this_thread::sleep_for(3s);
    return 0;
}
