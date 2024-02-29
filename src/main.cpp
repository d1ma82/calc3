#include <iostream>
#include <vector>
#include "service.h"

static bool yes() {

    char ch;
    std::cin.clear();
    std::cin.sync();
    std::cin >> ch;

    return ch == 'y';
}

int main () {

    try {
        
        do { 
            analize::Service srv(std::cin);
            std::cout<<'>';
            srv.run();            
            std::cout<<">Continue? y/n ";
        } while (yes());

    } catch (std::exception const &e) {
            LOGE("%s", e.what())
    }
    return 0;
}