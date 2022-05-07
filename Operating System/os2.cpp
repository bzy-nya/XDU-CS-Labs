#include<thread>
#include<mutex>
#include<bits/stdc++.h>

static int test = 1;
std::mutex mt;

void thread1(int n) {
    while(test <= n) {
        if( mt.try_lock() ) {
            std::cout << "Thread1 " << test << "\n";
            sleep(0.1); test ++;
            mt.unlock();
        }
    }
}

int main() {
    int n = 20;
    std::thread t1(thread1, n);
    while(test <= n) {
        if( mt.try_lock() ) {
            std::cout << "MainThread " << test << "\n";
            test ++;
            mt.unlock();
        }
    }
    return 0;
}
