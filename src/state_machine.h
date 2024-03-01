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

        bool next() { return handle? (handle.resume(), !handle.done()): false; }
        T current() const { return handle.promise().current_value; }

        generator (std::coroutine_handle<promise_type> h): handle{h} {}
        generator (generator const&)=delete;
        generator (generator && rhs): handle{ std::exchange(rhs.handle, nullptr) } {}
        ~generator() { if (handle) handle.destroy(); }
        private: std::coroutine_handle<promise_type> handle;
    };

    struct resumable {

        struct promise_type {
            auto get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
            auto initial_suspend() { return std::suspend_always(); }
            auto final_suspend() noexcept { return std::suspend_always(); }
            void return_void() {}
            void unhandled_exception() { std::rethrow_exception(std::current_exception()); }
        };

        resumable (std::coroutine_handle<promise_type> h): handle{h} {}
        resumable (resumable const&)=delete;
        resumable (resumable && rhs): handle{ std::exchange(rhs.handle, nullptr) } {}
        ~resumable() { if (handle) handle.destroy(); }

        auto get_handle() { return std::exchange(handle, nullptr); }
        private: std::coroutine_handle<promise_type> handle;
    };        

    template <typename F, typename Kind, typename SM> struct stm_awaiter : private F {
  
        SM &sm;
        stm_awaiter(F f, SM &sm) : F{f}, sm{sm} {}
        constexpr bool await_ready() const noexcept { return false; }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept {
        
            sm.gen_next();
            auto kind = sm.gen_current();
            auto newstate = F::operator()(kind);
            return sm[newstate];
        }

        bool await_resume() noexcept { return (sm.gen_current() != Kind::END); }
    };

    template<typename Context, typename State, typename Kind> class state_machine {
        Context& ctx;
        std::unordered_map<State, std::coroutine_handle<>> routines;
        generator<Kind> gen;

    public:
        state_machine (Context& ctx, generator<Kind> && g): ctx{ctx}, gen{ std::move(g) } {}

        Context* operator -> () const { return std::addressof(ctx); }
        void run (State first) { routines[first].resume(); }
        template<typename F> void add_state (State x, F func) { routines[x]=func(*this).get_handle(); }
        std::coroutine_handle<> operator [] (State s) { return routines[s]; }
        template <typename F> auto awaiter (F transition) { return stm_awaiter<F, Kind, decltype(*this)>(transition, *this); }
        Kind gen_current() const { return gen.current(); }
        void gen_next() { gen.next(); }
    };
}