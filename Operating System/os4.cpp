#include<unistd.h>
#include<bits/stdc++.h>


int main() {
    auto fa = getpid();
    
    int _pipe[2];
    int ret = pipe(_pipe);
    if(ret < 0) {
        std::cout << "pipe error\n";
        return 0;
    }
    
    auto x = fork();
    
    if( x == 0 ) {
        close(_pipe[1]);
        char s[100];
        memset(s, '\0', sizeof(s));
        read(_pipe[0], s, sizeof(s));
        std::cout << "fa : " << getpid() <<  " read " << s << "\n";
    } else {
        close(_pipe[0]);
        char* msg = NULL;
        msg = "awawawa";
        write(_pipe[1], msg, strlen(msg));
        std::cout << "ch : " << getpid() << " write successfully\n";
    } 
    return 0;
}
