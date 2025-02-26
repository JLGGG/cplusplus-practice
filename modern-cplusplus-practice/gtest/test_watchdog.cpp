#include <gtest/gtest.h>
#include "watchdog.hpp"

class TestWatchdog : public ::testing::Test {
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

static void exampleTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST_F(TestWatchdog, v1) {
    Watchdog wd(100);

    wd.registerTask(1, 1000);
    wd.registerTask(2, 1500);

    wd.addDependency(2, 1);

    std::thread taskThread([&wd](){
        for (int i = 0; i < 10; i++) {
            exampleTask();
            wd.feed(1);
            std::cout << "[Task 1] Execution cycle " << i << std::endl;
        }
    });

    std::thread taskThread2([&wd](){
        for (int i = 0; i < 10; i++) {
            exampleTask();
            wd.feed(2);
            std::cout << "[Task 2] Execution cycle " << i << std::endl;
        }
    });

    taskThread.join();
    taskThread2.join();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Main finished." << std::endl;
}
