#pragma once
#include <iostream>
#include <limits>
#include "log.h"

namespace token {    
    template<typename T> struct Scalar {
        T s {};
        Scalar() {}
        explicit Scalar (T const& v): s{v} {}
        //T const& operator [] (size_t) const { return s; }
        //T& operator [] (size_t) { return s; }      
        
        template<typename Ret> Scalar<Ret> neg () { return Scalar<Ret>(-s); }
        template<typename Ret, typename S> Scalar<Ret> add  (S b) { return Scalar<Ret>(s + b.s); }
        template<typename Ret, typename S> Scalar<Ret> diff (S b) { return Scalar<Ret>(s - b.s); }
        template<typename Ret, typename S> Scalar<Ret> mult (S b) { return Scalar<Ret>(s * b.s); }
        template<typename Ret, typename S> Scalar<Ret> div  (S b) { return Scalar<Ret>(b.s==0? std::numeric_limits<Ret>::infinity(): s / b.s); }
    };

    template<typename T> std::ostream& operator << (std::ostream& out, Scalar<T> const& sc) {

        out<<sc.s;
        return out;
    }
}