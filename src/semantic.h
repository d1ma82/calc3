#pragma once

#include <vector>
#include <stack>
#include <limits>
#include <algorithm>
#include <sstream>
#include <functional>
#include "arena.h"
#include "scalar.h"
#include "exec.h"
#include "log.h"

namespace analize {

    using execute = std::function<void (size_t)>;

    constexpr int priority(char op) {
    
        switch (op) {
            case 'u': return 2; // unar minus
            case '*': return 3;   
            case '/': return 3;            
            case '+': return 4;
            case '-': return 4;   
            case '=': return 5;
            case '(': return 6;
            default: return 0;
        }
    }
    
    struct command_t {
        size_t start;
        int priority {};
        execute run;
        void operator () () { run(start); }
    };

    class Semantic: private executor::Exe {
       
        std::vector<command_t> command_vec;
        std::stack<command_t> command_stack;
        std::stack<std::string> vars;

        int current_pr=0, old_pr=0; 

        void push (char value) {

            switch(value) {
                case '+': { command_stack.emplace( 2, priority('+'), [this] (size_t v) { executor::Exe::do_op<'+'>(v); } );break; }
                case '-': { command_stack.emplace( 2, priority('-'), [this] (size_t v) { executor::Exe::do_op<'-'>(v); } );break; }
                case '*': { command_stack.emplace( 2, priority('*'), [this] (size_t v) { executor::Exe::do_op<'*'>(v); } );break; }
                case '/': { command_stack.emplace( 2, priority('/'), [this] (size_t v) { executor::Exe::do_op<'/'>(v); } );break; }
                case 'u': { command_stack.emplace( 1, priority('u'), [this] (size_t v) { executor::Exe::do_op<'u'>(v); } );break; }
                case '=': { command_stack.emplace( 2, priority('='), [this] (size_t)   { executor::Exe::assign(); } );break; }
            }
        }         

        void old_lower() {

            while (!command_stack.empty() && command_stack.top().priority<=current_pr) {    

                command_vec.push_back(command_stack.top());
                command_stack.pop();
            }
        }            

    public:
        Semantic(): executor::Exe() {}

        template <typename T, symtab::RTTI type> void scalar(T value) {

            mem::arena.emplace_back (token::Scalar<T>(value), type);
            command_vec.emplace_back (mem::arena.size()-1, 0, [this] (size_t v) { executor::Exe::push(v); });
        }    

        void variable (std::string const& name) {

            if (mem::var_map.contains (name)) {

                command_vec.emplace_back (std::string::npos, 0, [this, name] (size_t) { executor::Exe::push_var_value(name); });
            
            } else {
                vars.push(name);
                mem::var_map[name]=std::string::npos;
                command_vec.emplace_back (0, 0, [this, name] (size_t) { executor::Exe::push_var(name); });
            }
        }

        void terminal (char value) {

            if (value=='=') vars.pop();
            
            current_pr = priority(value);
            old_pr     = command_stack.empty()? 0: command_stack.top().priority;

            if (command_stack.empty()) push(value);
            else if (old_pr>0 && current_pr>0 && old_pr<=current_pr) { old_lower(); push(value); }
            else push (value);
        }
    
        void end_equation() {
        
            while (!command_stack.empty()) {

                command_vec.push_back(command_stack.top());
                command_stack.pop();
            }
        }

        void open_bracket() {

            command_stack.emplace( 0, priority('('), nullptr );
        }   

        void close_bracket() {

            while (!command_stack.empty() && command_stack.top().priority != priority('(')) {

                command_vec.push_back(command_stack.top());
                command_stack.pop();
            }
            command_stack.pop();
        }        
        
        void run(bool syntax) {

            if (!vars.empty()) {
               
                std::stringstream str;
                str<<"Undefined variables ";

                while (!vars.empty()) {

                    str<<vars.top()<<',';
                    vars.pop(); 
                }
               
                std::cout<<str.str()<<'\n';
                return;
            }

            if (!syntax) {
                std::cout<<"syntax " << (syntax? "true\n":"false\n");
                return;
            } 

            std::for_each(command_vec.begin(), command_vec.end(), [] (command_t com) {com();});
            executor::Exe::print();            
        }        
    };
}