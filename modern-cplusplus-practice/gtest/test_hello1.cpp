#include <gtest/gtest.h>
#include "hello1.hpp"

class TestHello1 : public ::testing::Test {
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

TEST_F(TestHello1, hello) {
    fmt::print("fmt hello world");
}