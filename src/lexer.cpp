#include "lexer.h"
#include <set>
#include <string>
#include <algorithm>
#include "semantic.h"

namespace analize {

    enum State {S_UNDEF, S_INPUT, S_SCALAR, S_OPERATOR, S_SPECIAL};
    
    const std::set<char> operators {'+', '-', '*', '/'};
    const std::set<char> digits {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};
    const std::set<char> special {';', '!', '(', ')'};
    std::string buffer;
    State old_state, state;
    bool is_float;

    Lexic::Lexic(): Syntax(), Semantic() {

        syntax=true;
        state = S_INPUT, old_state=S_UNDEF;
        buffer="";
        is_float=false;
    }

    void Lexic::on_scalar (char ch) {

        state = S_INPUT, old_state=S_SCALAR;
        buffer += ch;
        if (ch=='.') is_float=true;
    }

    void Lexic::end_scalar() {

        bool ret = Syntax::non_terminal(W_SCALAR);
        if (!ret) syntax=ret;
       
        if (is_float) Semantic::scalar<double, symtab::SCALAR_FLOAT> (std::stod(buffer));
        else Semantic::scalar<int, symtab::SCALAR_INT>(std::stoi(buffer));

        buffer.clear();
        state=S_INPUT, old_state=S_INPUT;        
    }

    void Lexic::on_operator (char ch) {

        bool ret = Syntax::terminal(char_to_word(ch));
        if (!ret) syntax=ret;

        if (ch=='-' && (old_state==S_UNDEF || old_state==S_OPERATOR || old_state==S_SCALAR)) ch='u';
        Semantic::terminal(ch);
        state=S_INPUT, old_state=S_OPERATOR;
    }

    void Lexic::on_special(char ch) {
        
        if (ch!='!') {
            bool ret = Syntax::terminal(char_to_word(ch));
            if (!ret) syntax=ret;
        }

        switch (ch){
            case ';': Semantic::end_equation(); break;
            case '!': Semantic::run(syntax); break;
            case '(': Semantic::open_bracket(); break;
            case ')': Semantic::close_bracket(); break;
        }
        state=S_INPUT, old_state=S_SPECIAL; 
    }

    void Lexic::on_input (char ch) {

                // Scalars
        state = std::any_of(digits.cbegin(), digits.cend(), 
                    [=] (const char c) { return ch==c; })? S_SCALAR: S_INPUT;

        if (state==S_SCALAR) { on_scalar(ch); return; }
        if (old_state==S_SCALAR) { end_scalar(); on_input(ch); return; }
                

                // Operators
        state = std::any_of(operators.cbegin(), operators.cend(), 
            [=] (const char c) { return ch==c; })? S_OPERATOR: S_INPUT;
        if (state==S_OPERATOR) { on_operator(ch); return; }


        state = std::any_of(special.begin(), special.end(), 
            [=](const char c) { return ch==c; })? S_SPECIAL: S_INPUT;
        if (state==S_SPECIAL) { on_special(ch); return; }

        LOGD("Unknown symbol %c", ch)
    }   

    Lexic& Lexic::operator << (char ch) {

        if (state==S_INPUT) on_input(ch);
        return *this;
    }
}