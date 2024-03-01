#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include "scalar.h"
#include "log.h"

namespace symtab {
    enum RTTI {UNDEF, SCALAR_INT, SCALAR_FLOAT};

    class object_t final {
        
        RTTI type;
        
        struct concept_t {

            virtual ~concept_t()=default;
            virtual std::unique_ptr<concept_t> copy_() const = 0;
            virtual void print() const = 0;
        };

        template<typename T> struct model final: public concept_t {

            T data;
            model (T x): data(std::move(x)) {}
            std::unique_ptr<concept_t> copy_() const override { return std::make_unique<model>(*this);}
            void print() const { std::cout<<data; }
        };
        std::unique_ptr<concept_t> self;
    public:
        
        RTTI is() const { return type; }
        template<typename T> object_t(T x, RTTI type)
            : self (std::make_unique<model<T>>(std::move(x))), type{type} {}

        object_t(object_t const& x)=delete;
        object_t(object_t&& x) noexcept =default;
  

        template <typename T> T const& get () const {

            auto* instance = dynamic_cast<model<T>*>(self.get());
            if (instance == nullptr) throw std::runtime_error("Cannot aquire instance");
            return instance->data; 
        }

        static void print (std::vector<symtab::object_t>::const_iterator& x) { x->self->print(); }
    };
}