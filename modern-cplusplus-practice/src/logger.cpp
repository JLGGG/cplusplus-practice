#include "logger.hpp"

Logger::Logger() {
    thread_ = std::thread {&Logger::ProcessEntries, this};
}