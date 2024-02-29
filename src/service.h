#pragma once
#include <functional>
#include "token.h"
#include "syntax.h"
#include "semantic.h"
#include "state_machine.h"

namespace analize {

    enum State {S_INPUT, S_SCALAR, S_OPERATOR, S_SPECIAL};
    enum Kind {UNDEF, INPUT, OPERATOR, SPECIAL, DIGIT, END};

    class Service { 
    private:
        stm::state_machine<State, Kind> sm;
    public:
        Service (std::istream& istr);
        void run() { sm.run(S_INPUT); }
    };
}