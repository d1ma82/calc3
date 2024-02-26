#include <iostream>
#include <vector>
#include "lexer.h"

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
            analize::Lexic lex;
            std::cout<<'>';
            for (char ch; std::cin>>ch;) {

                lex<<ch;
                if (ch=='!') break;
            }
            
            std::cout<<">Continue? y/n ";
        } while (yes());

    } catch (std::exception const &e) {
            LOGE("%s", e.what())
    }
    return 0;
}