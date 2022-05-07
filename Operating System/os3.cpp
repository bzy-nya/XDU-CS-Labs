#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<bits/stdc++.h>

void sig_handle(int sig) {
    std::cout << "get sigint\n";
    signal(SIGINT, SIG_DFL);
}

int main() {
    auto fa = getpid();
    auto x = fork();
    if( getpid() == fa ) {
        std::cout << "fa : " << getpid() << "\n";
        if(signal(SIGINT, sig_handle) == SIG_ERR) {
            std::cout << "qwq\n";
        }
        sleep(100);
    } else {
        std::cout << "ch : " << getpid() << "\n";
        kill( getppid(), SIGINT );
    } 
}
