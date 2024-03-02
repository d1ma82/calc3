#pragma once
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include "log.h"

namespace analize {
    
    enum Word { W_UNDEF, W_SCALAR, W_VARIABLE, W_PLUS, W_MINUS, W_MULT, W_DIV, W_ASSIGN, W_OPEN_BRACKET, W_CLOSE_BRACKET, W_END_EQUATION, W_E, W_P, W_K, COUNT };
    typedef  std::queue<Word> Rule;

    Word char_to_word(char ch);

    class Syntax {
    private:
        Rule start;
        std::stack<Rule> rules;
        Rule table[3][Word::COUNT-3];
        
        void init_table();
        std::string rule_to_string(Rule rule);

        bool check(Word input);
    public:
        Syntax() { start.push(W_E); init_table(); }

        bool non_terminal (Word word) {

            if (rules.empty()) rules.push (start);
            return word==W_UNDEF? false: check (word);
        }

        bool terminal (Word word) {
            
            if (rules.empty()) rules.push (start);
            return word==W_UNDEF? false: check (word);
        }
    };
}