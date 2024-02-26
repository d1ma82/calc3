#pragma once
#include "syntax.h"
#include "semantic.h"

namespace analize {
    
    class Lexic final: private Syntax , private Semantic{ 
    private:
        bool syntax {true};
        void on_scalar (char ch);
        void end_scalar();
        void on_operator (char ch);
        void on_special(char ch);
        void on_input(char ch);
    public:
        Lexic ();
        Lexic& operator << (char ch);
    };


}