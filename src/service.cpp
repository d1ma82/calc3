#include "service.h"
#include <set>
#include <string>
#include <algorithm>
#include "semantic.h"

namespace analize {
    
    std::set<char> char_set;
    const std::set<char> operators {'+', '-', '*', '/', '='};
    const std::set<char> digits {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};
    const std::set<char> special {';', '!', '(', ')'};
    const std::set<char> unar_minus_special {';', '('};
    const auto transition {
        [] (auto kind) {

            switch (kind) {
                case DIGIT:     return S_SCALAR;
                case CHAR:      return S_VARIABLE;
                case SPECIAL:   return S_SPECIAL;
                case OPERATOR:  return S_OPERATOR;
                default:        return S_INPUT;
            }
        }
    };      

    stm::generator<Kind> Service::input(std::istream& istr) {

        Kind kind=INPUT, old=UNDEF;

        for (char ch; istr>>ch;) {
            
            char old_special='\0';
            kind = digits.contains(ch)? DIGIT: INPUT;

            if (kind==DIGIT) { 

                if (ch=='.') is_float=true;
                buff+=ch; old=kind; kind=INPUT; continue; 
            }

            if (old==DIGIT)  { 

                co_yield old; kind=INPUT, old=INPUT; 
                buff.clear(); istr.putback(ch); continue; 
            }

            kind = char_set.contains(ch)? CHAR: INPUT;
            
            if (kind==CHAR) {
                buff+=ch; old=kind; kind=INPUT; continue;
            } 

            if (old==CHAR) {
                co_yield old; kind=INPUT, old=INPUT;
                buff.clear(); istr.putback(ch); continue;
            }

            kind = operators.contains(ch)? OPERATOR: INPUT;
            
            if (kind==OPERATOR) { 
                
                if (ch=='-' && (old==UNDEF || old==OPERATOR || unar_minus_special.contains(old_special))) ch='u';
                buff+=ch; co_yield kind; kind=INPUT, old=OPERATOR; buff.clear(); continue; 
            }

            kind = special.contains(ch)? SPECIAL: INPUT;
            
            if (kind==SPECIAL) { 

                old_special=ch, buff+=ch; 
                co_yield kind; kind=INPUT, old=SPECIAL; buff.clear(); 
                if (ch!='!') continue; 
            }

            co_yield END;
        } 
    }

    stm::resumable State_INPUT (Service&, stm::state_machine<Service, State, Kind>& stm) {

        while (co_await stm.awaiter(transition));    
    }

    stm::resumable State_VARIABLE (Service& srv, stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret = srv.Syntax::non_terminal(W_VARIABLE);
            if (!ret) srv.synt_ok=false;

            srv.Semantic::variable(srv.buff);

        } while (co_await stm.awaiter(transition));
    }

    stm::resumable State_SCALAR (Service& srv, stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret = srv.Syntax::non_terminal(W_SCALAR);
            if (!ret) srv.synt_ok=false;
       
            if (srv.is_float) { srv.Semantic::scalar<double, symtab::SCALAR_FLOAT> (std::stod(srv.buff)); srv.is_float=false; }
            else srv.Semantic::scalar<int, symtab::SCALAR_INT>(std::stoi(srv.buff));
    
        } while (co_await stm.awaiter(transition));
    }

    stm::resumable State_OPERATOR (Service& srv, stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret = srv.Syntax::terminal(char_to_word(srv.buff[0]));
            if (!ret) srv.synt_ok=false;
            
            srv.Semantic::terminal(srv.buff[0]);
        
        } while (co_await stm.awaiter(transition));
    }

    stm::resumable State_SPECIAL (Service& srv, stm::state_machine<Service, State, Kind>& stm) {

        do {
            bool ret=true;

            switch (srv.buff[0]){
                case '!': srv.Semantic::run(srv.synt_ok); break;
                case ';': { ret = srv.Syntax::terminal(char_to_word(srv.buff[0])); srv.Semantic::end_equation(); break; }
                case '(': { ret = srv.Syntax::terminal(char_to_word(srv.buff[0])); srv.Semantic::open_bracket(); break; }
                case ')': { ret = srv.Syntax::terminal(char_to_word(srv.buff[0])); srv.Semantic::close_bracket(); break;}
            }
            if (!ret) srv.synt_ok=ret;
        
        } while (co_await stm.awaiter(transition));    
    }        

    Service::Service(std::istream& istr): sm{*this, input(istr)} {

        buff     = "";
        synt_ok  = true;
        is_float = false;

        if (char_set.empty()) {
            for (char c='a'; c<='z'; ++c) {
                
                char_set.insert(c);
                char_set.insert(std::toupper(c));
            }
        }

        mem::arena.clear();
        mem::var_map.clear();

        sm.add_state(S_INPUT, State_INPUT);
        sm.add_state(S_SCALAR, State_SCALAR);
        sm.add_state(S_VARIABLE, State_VARIABLE);
        sm.add_state(S_OPERATOR, State_OPERATOR);
        sm.add_state(S_SPECIAL, State_SPECIAL);
    }
}