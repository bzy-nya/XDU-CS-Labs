#include<unistd.h>
#include<iostream>
#include<fstream>

int qwq = 0;
std::string awa = "";

int main() {
    auto x = vfork();
    if( x == 0 ) {
        std::ifstream in("os1.in");
        in >> awa; 
        in.close();
        qwq = 1;
        std::cout << "child ended\n";
        
    } else {
        while( !qwq ) ;
        std::cout << "main proecess run\n";
        std::cout << awa << "\n";
        std::cout << "main proecess ended\n";
    }
    return EXIT_SUCCESS;
}
