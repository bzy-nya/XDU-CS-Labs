#include<unistd.h>
#include<sys/stat.h>
#include<cstring>
#include<iostream>
#include<fstream>

#define PATH "/tmp/tmp_fifo.tmp"

int main() {
    auto r = mkfifo( PATH, S_IFIFO | 0666 );
    auto x = fork();
    if( x == 0 ) {
        std::ofstream out(PATH);
        for( int i = 1; i <= 10; i ++ ) {
            out << i << "\n" << std::flush;
            std::cout << "child " << getpid() << " write " << i << "\n";
            sleep(1);
        }
        out << "END\n" << std::flush;
        std::cout << "child ends\n";
    } else {
        std::ifstream in(PATH);
        std::string s;
        while( getline(in, s) and s != "END" ) {
            std::cout << "father " << getpid() << " read " << s << "\n";
        }
        std::cout << "father ends\n";
    }
}
