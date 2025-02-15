#include <fmt/core.h>
#include <fmt/format.h>

#include <thread>
#include <stdexcept>

#include <atomic>

using namespace std;

void DoSomeWork() {
    for (int i {0}; i < 5; i++) {
        fmt::print("{}\n", i);
    }
    fmt::print("Thread throwing a runtime_error exception...\n");
    throw runtime_error {"Exception from thread"};
}

void ThreadFunc(exception_ptr& err) {
    try {
        DoSomeWork();
    } catch (...) {
        fmt::print("Thread caught exception, returning exception...\n");
        err = current_exception();
    }
}

void DoWorkInThread() {
    exception_ptr error;
    thread t {ThreadFunc, ref(error)};
    t.join();

    if (error) {
        fmt::print("Main thread received exception, rethrowing it...\n");
        rethrow_exception(error);
    } else {
        fmt::print("Main thread did not receive any exception\n");
    }
}

void Increment(int& counter) {

    atomic_ref<int> atomic_counter {counter};
    int result {0};
    for (int i {0}; i < 100; i++) {
        ++result;
        this_thread::sleep_for(1ms);
    }
    atomic_counter += result;
}