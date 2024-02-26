#pragma once
#include <stack>
#include <cassert>
#include "arena.h"
#include "scalar.h"

namespace executor {

    class Exe {

        std::stack<size_t> run_st;

        template<typename R, typename F, typename S> void dbl(
                char op,
                symtab::RTTI rtti,
                std::vector<symtab::object_t>::const_iterator& ita,
                std::vector<symtab::object_t>::const_iterator& itb
        ) {
            auto first = ita->get<F>();
            auto second = itb->get<S>();
            switch (op) {
                case '+': {mem::arena.emplace_back ( first.template add<R, S>(second), rtti);  break;}
                case '-': {mem::arena.emplace_back ( first.template diff<R, S>(second), rtti); break;}
                case '*': {mem::arena.emplace_back ( first.template mult<R, S>(second), rtti); break;}
                case '/': {mem::arena.emplace_back ( first.template div<R, S>(second), rtti);  break;}
            }
            run_st.push(mem::arena.size()-1); 
        }

        template<typename R, typename F> void unar(
                char op,
                symtab::RTTI rtti,
                std::vector<symtab::object_t>::const_iterator& ita
        ) {
            auto first = ita->get<F>();
            switch (op) {
                case 'u': {mem::arena.emplace_back ( first.template neg<R>(), rtti); break;}
            }
            run_st.push(mem::arena.size()-1); 
        }    

        void internal( 
                char op,
                std::vector<symtab::object_t>::const_iterator& ita,
                std::vector<symtab::object_t>::const_iterator& itb
        ) {
            if (ita==itb) {

                std::cout<<"Expr:\t"<<op; symtab::object_t::print(ita);std::cout<<'\n';
                switch (ita->is()) {
                    case symtab::SCALAR_INT:    unar<int, token::Scalar<int>>(op, ita->is(), ita); break;
                    case symtab::SCALAR_FLOAT:  unar<double, token::Scalar<double>>(op, ita->is(), ita); break;
                    default: goto error;
                }
            } else if (ita->is() == symtab::SCALAR_INT) {

                std::cout<<"Expr:\t";symtab::object_t::print(ita);std::cout<<op;symtab::object_t::print(itb);std::cout<<'\n';
                switch (itb->is()) {
                    case symtab::SCALAR_INT: dbl<int, token::Scalar<int>, token::Scalar<int>>(op, ita->is(), ita, itb); break;
                    case symtab::SCALAR_FLOAT: dbl<double, token::Scalar<int>, token::Scalar<double>>(op, itb->is(), ita, itb); break;
                    default: goto error;
                }            
            } else if (ita->is() == symtab::SCALAR_FLOAT) {

                std::cout<<"Expr:\t";symtab::object_t::print(ita);std::cout<<op;symtab::object_t::print(itb);std::cout<<'\n';
                switch (itb->is()) {
                    case symtab::SCALAR_INT: dbl<double, token::Scalar<double>, token::Scalar<int>>(op, ita->is(), ita, itb); break;
                    case symtab::SCALAR_FLOAT: dbl<double, token::Scalar<double>, token::Scalar<double>>(op, ita->is(), ita, itb); break;
                    default: goto error;
                }             
            } else goto error;
            return;
            error: throw std::runtime_error("Unknown operands");
        }

    public:
        void push(size_t start) { run_st.push(start); }
        
        template<char op> void do_op ( size_t count ) {

            auto itb = std::next(mem::arena.cbegin(), run_st.top());
            run_st.pop();
            if (count==1) { internal(op, itb, itb); return; }

            auto ita = std::next(mem::arena.cbegin(), run_st.top());
            run_st.pop(); 
            internal(op, ita, itb);        
        }   

        void print() { 

            if (run_st.empty()) { std::cout<<"0.0\n"; return; }
            auto it = std::next(mem::arena.cbegin(), run_st.top());
            symtab::object_t::print(it);std::cout<<'\n';
        }
    };
}