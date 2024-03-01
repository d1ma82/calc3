#include "service.h"
#include <set>
#include <string>
#include <algorithm>
#include "semantic.h"

namespace analize {
    
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

    stm::generator<Kind> Service::input(std::istream& istr) {

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

    stm::resumable State_INPUT (stm::state_machine<Service, State, Kind>& stm) {

        while (co_await stm.awaiter(transition));    
    }

    stm::resumable State_SCALAR (stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret = stm->Syntax::non_terminal(W_SCALAR);
            if (!ret) stm->synt_ok=false;
       
            if (stm->is_float) { stm->Semantic::scalar<double, symtab::SCALAR_FLOAT> (std::stod(stm->buff)); stm->is_float=false; }
            else stm->Semantic::scalar<int, symtab::SCALAR_INT>(std::stoi(stm->buff));
    
        } while (co_await stm.awaiter(transition));
    }

    stm::resumable State_OPERATOR (stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret = stm->Syntax::terminal(char_to_word(stm->buff[0]));
            if (!ret) stm->synt_ok=false;
            
            stm->Semantic::terminal(stm->buff[0]);
        
        } while (co_await stm.awaiter(transition));
    }

    stm::resumable State_SPECIAL (stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret=true;

            switch (stm->buff[0]){
                case '!': stm->Semantic::run(stm->synt_ok); break;
                case ';': { ret = stm->Syntax::terminal(char_to_word(stm->buff[0])); stm->Semantic::end_equation(); break; }
                case '(': { ret = stm->Syntax::terminal(char_to_word(stm->buff[0])); stm->Semantic::open_bracket(); break; }
                case ')': { ret = stm->Syntax::terminal(char_to_word(stm->buff[0])); stm->Semantic::close_bracket(); break;}
            }
            if (!ret) stm->synt_ok=ret;
        
        } while (co_await stm.awaiter(transition));    
    }        

    Service::Service(std::istream& istr): sm{*this, input(istr)} {

        buff     = "";
        synt_ok  = true;
        is_float = false;

        sm.add_state(S_INPUT, State_INPUT);
        sm.add_state(S_SCALAR, State_SCALAR);
        sm.add_state(S_OPERATOR, State_OPERATOR);
        sm.add_state(S_SPECIAL, State_SPECIAL);
    }
}