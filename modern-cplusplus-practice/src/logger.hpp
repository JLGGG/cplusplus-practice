#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>>

class Logger {
public:
    Logger();
    Logger(const Logger& src) = delete;
    Logger& operator=(const Logger& rhs) = delete;
    void log(std::string entry);

private:
    void ProcessEntries();
    void ProcessEntriesHelper(std::queue<std::string>& queue, std::ofstream& ofs) const;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::queue<std::string> queue_;
    std::thread thread_;
};

#endif // LOGGER_HPP
