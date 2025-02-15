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