#include <gtest/gtest.h>
#include "my_thread_test.hpp"

class TestThread : public ::testing::Test {
protected:
    void SetUp() override {
    // Code here will be called immediately after the constructor (right
    // before each test).
    }

    void TearDown() override {
    // Code here will be called immediately after each test (right
    // before the destructor).
    }
};

TEST_F(TestThread, exception) {
    try {
        DoWorkInThread();
    } catch (const exception& e) {
        fmt::print("Main function caught: {}\n", e.what());
    }
}

TEST_F(TestThread, atomic) {
    int counter {0};
    vector<thread> threads;

    for (int i {0}; i < 10; i++) {
        threads.push_back(thread {Increment, ref(counter)});
    }

    for (auto& t : threads) {
        t.join();
    }
    fmt::print("Result = {}\n", counter);
}

TEST_F(TestThread, call_once) {
    vector<thread> threads {3};
    for (auto& t : threads) {
        t = thread {ProcessingFunction};
    }

    for (auto& t : threads) {
        t.join();
    }
}

TEST_F(TestThread, spinlock) {
    vector<size_t> data;
    vector<thread> threads;
    for (size_t i {0}; i<NumberOfThreads; ++i) {
        threads.push_back(thread {dowork, i, ref(data)});
    }
    for (auto& t : threads) {
        t.join();
    }
    fmt::print("data contains {} elements, expected {}.\n", data.size(), NumberOfThreads * LoopsPerThread);
}

TEST_F(TestThread, packaged_task) {
    packaged_task<int(int, int)> task {CalculateSum};
    auto my_future {task.get_future()};
    thread my_thread {move(task), 100, 100};

    fmt::print("Test packaged_task\n");

    auto async_future {async(CalculateSum, 1000, 1000)};

    int result {my_future.get()};
    int async_result {async_future.get()};
    fmt::print("Packaged_task Result: {}\n", result);
    fmt::print("Async Result: {}\n", async_result);

    my_thread.join();
}