#pragma once
#include <functional>
#include "token.h"
#include "syntax.h"
#include "semantic.h"
#include "state_machine.h"

namespace analize {

    enum State {S_INPUT, S_SCALAR, S_VARIABLE, S_OPERATOR, S_SPECIAL};
    enum Kind {UNDEF, INPUT, OPERATOR, SPECIAL, DIGIT, CHAR, END};

    class Service: private Syntax, private Semantic { 
    private:
        bool is_float;
        bool synt_ok;
        std::string buff;  
        stm::state_machine<Service, State, Kind> sm;

        stm::generator<Kind> input(std::istream& istr);
        friend stm::resumable State_VARIABLE (Service& srv, stm::state_machine<Service, State, Kind>&);
        friend stm::resumable State_SCALAR (Service& srv, stm::state_machine<Service, State, Kind>&);
        friend stm::resumable State_INPUT (Service& srv, stm::state_machine<Service, State, Kind>&);
        friend stm::resumable State_OPERATOR (Service& srv, stm::state_machine<Service, State, Kind>&);
        friend stm::resumable State_SPECIAL (Service& srv, stm::state_machine<Service, State, Kind>&) ;
    public:
        Service (std::istream& istr);
        void run() { sm.run(S_INPUT); }
    };
}