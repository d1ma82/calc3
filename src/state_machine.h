#include <coroutine>
#include <utility>
#include <exception>
#include <unordered_map>

namespace stm {

    template<typename T> struct generator {

        struct promise_type {
            T current_value;
            auto get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
            auto initial_suspend() { return std::suspend_always(); }
            auto final_suspend() noexcept { return std::suspend_always(); }
            void return_void() {}
            void unhandled_exception() { std::rethrow_exception(std::current_exception()); }
            auto yield_value(T value) { current_value = value; return std::suspend_always(); }
        };

        bool next() { return handle_? (handle_.resume(), !handle_.done()): false; }
        T current() const { return handle_.promise().current_value; }

        generator (std::coroutine_handle<promise_type> h): handle_{h} {}
        generator (generator const&)=delete;
        generator (generator && rhs): handle_{ std::exchange(rhs.handle_, nullptr) } {}
        ~generator() { if (handle_) handle_.destroy(); }
        private: std::coroutine_handle<promise_type> handle_;
    };

    struct resumable {

        struct promise_type {
            auto get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
            auto initial_suspend() { return std::suspend_always(); }
            auto final_suspend() noexcept { return std::suspend_always(); }
            void return_void() {}
            void unhandled_exception() { std::rethrow_exception(std::current_exception()); }
        };

        resumable (std::coroutine_handle<promise_type> h): handle_{h} {}
        resumable (resumable const&)=delete;
        resumable (resumable && rhs): handle_{ std::exchange(rhs.handle_, nullptr) } {}
        ~resumable() { if (handle_) handle_.destroy(); }

        bool resume() { if (!handle_.done()) handle_.resume(); return !handle_.done(); }
        auto handle() { return std::exchange(handle_, nullptr); }
        private: std::coroutine_handle<promise_type> handle_;
    };        

    template <typename F, typename Kind, typename SM> struct stm_awaiter : private F {
  
        SM &stm_;
        stm_awaiter(F f, SM &stm) : F{f}, stm_{stm} {}
        constexpr bool await_ready() const noexcept { return false; }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept {
        
            stm_.gen_next();
            auto sym = stm_.gen_current();
            auto newstate = F::operator()(sym);
            return stm_[newstate];
        }

        bool await_resume() noexcept { return (stm_.gen_current() == Kind::END); }
    };

    template<typename State, typename Kind> class state_machine {

        std::unordered_map<State, std::coroutine_handle<>> routines_;
        generator<Kind> gen_;

    public:
        state_machine (generator<Kind> && g): gen_{ std::move(g) } {}

        void run (State first) { routines_[first].resume(); }
        template<typename F> void add_state (State x, F func) { routines_[x]=func(*this).handle(); }
        std::coroutine_handle<> operator [] (State s) { return routines_[s]; }
        template <typename F> auto awaiter (F transition) { return stm_awaiter<F, Kind, decltype(*this)>(transition, *this); }
        Kind gen_current() const { return gen_.current(); }
        void gen_next() { gen_.next(); }
    };
}