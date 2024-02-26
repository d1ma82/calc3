#include "syntax.h"
#include <sstream>


namespace analize {
    
    constexpr int E=0, P=1, K=2;

    Word char_to_word(char ch) {
        
        switch (ch) {
            case '+': return W_PLUS;
            case '*': return W_MULT;
            case '-': return W_MINUS;
            case '/': return W_DIV;
            case ';': return W_END_EQUATION;
            case '(': return W_OPEN_BRACKET;
            case ')': return W_CLOSE_BRACKET;
            default: return W_UNDEF;
        }
    }

    const char* word_to_str(Word word) {
        switch (word) {

            case  W_SCALAR:         return "SCALAR";
           // case  W_VECTOR:         return "VECTOR";
            //case  W_VAR:            return "VARIABLE";
            case  W_PLUS:           return "PLUS";
            case  W_MINUS:          return "MINUS";
            case  W_MULT:           return "MULTIPLY";
            case  W_DIV:            return "DIV";
            case  W_UNDEF:          return "UDEFINED";
            case W_END_EQUATION:    return "END_EQUATION";
            case  W_OPEN_BRACKET:   return "OPEN_BRACKET";
            case  W_CLOSE_BRACKET:  return "CLOSE_BRACKET";
            //case  W_ASSIGN:         return "ASSIGN";
            case  W_E:              return "E";
            case  W_P:              return "P";
            case  W_K:              return "K";
            default :               return "UNKNOWN";
        }
    }

    std::string Syntax::rule_to_string(Rule rule) {

        if (rule.size()==0) return "{}";

        std::stringstream ret; 
        ret << "{";
        ret << word_to_str(rule.front());
        if (rule.size()>1) ret << ',' << word_to_str(rule.back());
        ret << '}';
        return ret.str();        
    }

    bool Syntax::check(Word input) {

        Rule rule = rules.top();

            while (!rules.empty()) {

                rules.pop();
                //LOGI("Rule: %s\tType: %s", rule_to_string(rule).c_str(), word_to_str(input))

                Word word = rule.empty()? W_UNDEF: rule.front();
                if (!rule.empty()) rule.pop();

                switch (word) {
                    case W_UNDEF: return false;
                    case W_E:
                        rules.push(table[E][input]);
                        rule = rules.top();
                        if (rule.empty()) return false;
                        break;
                    
                    case W_P:
                        rules.push(table[P][input]);
                        rule = rules.top();
                        if (rule.empty()) return false;
                        break;

                    case W_K:
                        rules.push(table[K][input]);
                        rule = rules.top();
                        if (rule.empty()) return false;
                        break;       
                    
                    default:                                     
                        rules.push(rule);
                        return word==input;
                }
            }
            return false;
    }
    
    void Syntax::init_table() {
    // initialize rule set following table
//               c  vecP    var   +   -   *   /   ;  (    )    =    
//            E cP  vecP   varK      -E              (E                                
//            P                   +E -E   *E  /E  ;       )P      
//            K                   +E -E   *E  /E  ;       )P   =E
        table[E][W_SCALAR]        .push(W_SCALAR);
        table[E][W_SCALAR]        .push(W_P);
        //table[E][W_VECTOR]       .push(W_VECTOR);
        //table[E][W_VECTOR]       .push(W_P);
        //table[E][W_VAR]          .push(W_VAR);
       // table[E][W_VAR]          .push(W_K);   
        table[E][W_MINUS]        .push(W_MINUS);
        table[E][W_MINUS]        .push(W_E);
        //table[E][W_END_EQUATION] .push(W_END_EQUATION);
        table[P][W_END_EQUATION] .push(W_END_EQUATION);          
        table[P][W_PLUS]         .push(W_PLUS);
        table[P][W_PLUS]         .push(W_E);
        table[P][W_MINUS]        .push(W_MINUS);
        table[P][W_MINUS]        .push(W_E);
        table[P][W_MULT]      .push(W_MULT);
        table[P][W_MULT]      .push(W_E);
        table[P][W_DIV]          .push(W_DIV);
        table[P][W_DIV]          .push(W_E);
        table[E][W_OPEN_BRACKET] .push(W_OPEN_BRACKET);
        table[E][W_OPEN_BRACKET] .push(W_E);
        table[P][W_CLOSE_BRACKET].push(W_CLOSE_BRACKET);
        table[P][W_CLOSE_BRACKET].push(W_P);
        table[K][W_PLUS]         .push(W_PLUS);
        table[K][W_PLUS]         .push(W_E);   
        table[K][W_MINUS]        .push(W_MINUS);
        table[K][W_MINUS]        .push(W_E);
        table[K][W_DIV]          .push(W_DIV);
        table[K][W_DIV]          .push(W_E);
        table[K][W_MULT]      .push(W_MULT);
        table[K][W_MULT]      .push(W_E);
        table[K][W_END_EQUATION] .push(W_END_EQUATION);
        table[K][W_CLOSE_BRACKET].push(W_CLOSE_BRACKET);
        table[K][W_CLOSE_BRACKET].push(W_P);
        //table[K][W_ASSIGN]       .push(W_ASSIGN);
        //table[K][W_ASSIGN]       .push(W_E);
    }   
    
}