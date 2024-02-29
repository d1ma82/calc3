#include "service.h"
#include <set>
#include <string>
#include <algorithm>
#include "semantic.h"

namespace analize {
    
    bool is_float;
    bool synt_ok;
    std::string buff;  
    Syntax syntax;
    Semantic semantic;

    const std::set<char> operators {'+', '-', '*', '/'};
    const std::set<char> digits {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};
    const std::set<char> special {';', '!', '(', ')'};
    const auto transition {
        [] (auto kind) {

            switch (kind) {
                case DIGIT: return S_SCALAR;
                case SPECIAL: return S_SPECIAL;
                case OPERATOR: return S_OPERATOR;
                default: return S_INPUT;
            }
        }
    };      

    stm::generator<Kind> input(std::istream& istr) {

        Kind kind=INPUT, old=UNDEF;

        for (char ch; istr>>ch;) {

            kind = std::any_of(digits.cbegin(), digits.cend(), 
                    [=] (const char c) { return ch==c; })? DIGIT: INPUT;

            if (kind==DIGIT) { 

                if (ch=='.') is_float=true;
                buff+=ch; old=kind; kind=INPUT; continue; 
            }

            if (old==DIGIT)  { 

                co_yield old; kind=INPUT, old=INPUT; 
                buff.clear(); istr.putback(ch); continue; 
            } 

            kind = std::any_of(operators.cbegin(), operators.cend(), 
                    [=] (const char c) { return ch==c; })? OPERATOR: INPUT;
            
            if (kind==OPERATOR) { 
                
                if (ch=='-' && (old==UNDEF || old==OPERATOR || old==SPECIAL)) ch='u';
                buff+=ch; co_yield kind; kind=INPUT, old=OPERATOR; buff.clear(); continue; 
            }

            kind = std::any_of(special.begin(), special.end(), 
                    [=](const char c) { return ch==c; })? SPECIAL: INPUT;
            
            if (kind==SPECIAL) { 

                buff+=ch; co_yield kind; kind=INPUT, old=SPECIAL; buff.clear(); 
                if (ch!='!') continue; 
            }

            co_yield END;
        } 
    }

    stm::resumable State_INPUT (stm::state_machine<State, Kind>& stm) {

        while (!co_await stm.awaiter(transition));    
    }

    stm::resumable State_SCALAR (stm::state_machine<State, Kind>& stm) {

        do {
            bool ret = syntax.non_terminal(W_SCALAR);
            if (!ret) synt_ok=false;
       
            if (is_float) { semantic.scalar<double, symtab::SCALAR_FLOAT> (std::stod(buff)); is_float=false; }
            else semantic.scalar<int, symtab::SCALAR_INT>(std::stoi(buff));
    
        } while (!co_await stm.awaiter(transition));
    }

    stm::resumable State_OPERATOR (stm::state_machine<State, Kind>& stm) {

        do {
            bool ret = syntax.terminal(char_to_word(buff[0]));
            if (!ret) synt_ok=false;
            
            semantic.terminal(buff[0]);
        
        } while (!co_await stm.awaiter(transition));
    }

    stm::resumable State_SPECIAL (stm::state_machine<State, Kind>& stm) {

        do {
            bool ret=true;

            switch (buff[0]){
                case '!': semantic.run(synt_ok); break;
                case ';': { ret = syntax.terminal(char_to_word(buff[0])); semantic.end_equation(); break; }
                case '(': { ret = syntax.terminal(char_to_word(buff[0])); semantic.open_bracket(); break; }
                case ')': { ret = syntax.terminal(char_to_word(buff[0])); semantic.close_bracket(); break;}
            }
            if (!ret) synt_ok=ret;
        
        } while (!co_await stm.awaiter(transition));    
    }        

    Service::Service(std::istream& istr): sm{input(istr)} {

        buff     = "";
        synt_ok  = true;
        is_float = false;

        syntax = Syntax();
        semantic = Semantic();

        sm.add_state(S_INPUT, State_INPUT);
        sm.add_state(S_SCALAR, State_SCALAR);
        sm.add_state(S_OPERATOR, State_OPERATOR);
        sm.add_state(S_SPECIAL, State_SPECIAL);
    }
}