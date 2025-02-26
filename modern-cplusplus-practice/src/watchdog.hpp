#ifndef WATCHDOG_HPP
#define WATCHDOG_HPP

#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

typedef uint8_t TaskID;

struct TaskInfo {
    std::chrono::steady_clock::time_point last_feed;
    uint32_t threshold_ms;
};

class Watchdog {
private:
    std::map<TaskID, TaskInfo> tasks_;
    std::map<TaskID, std::vector<TaskID>> dependency_graph_;
    uint32_t check_interval_ms_;
    std::mutex mtx_;
    std::thread watchdog_thread_;
    volatile bool stop_flag_;
public:
    Watchdog(uint32_t check_interval_ms = 100)
    : check_interval_ms_(check_interval_ms), stop_flag_(false) {
        watchdog_thread_ = std::thread(&Watchdog::monitor, this);
    }

    ~Watchdog() {
        stop_flag_ = true;
        if (watchdog_thread_.joinable()) {
            watchdog_thread_.join();
        }
    }

    void registerTask(TaskID id, uint32_t threshold_ms) {
        std::lock_guard<std::mutex> lock(mtx_);
        TaskInfo info;
        info.last_feed = std::chrono::steady_clock::now();
        info.threshold_ms = threshold_ms;
        tasks_[id] = info;
    }

    void addDependency(TaskID task, TaskID dependsOn) {
        std::lock_guard<std::mutex> lock(mtx_);
        dependency_graph_[task].push_back(dependsOn);
    }

    void feed(TaskID id) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = tasks_.find(id);
        if (it != tasks_.end()) {
            it->second.last_feed = std::chrono::steady_clock::now();
            std::cout << "[Watchdog] Task " << id << "fed.\n";
        }
    }

private:
    void monitor() {
        while (!stop_flag_) {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                auto now = std::chrono::steady_clock::now();
                for (const auto& pair : tasks_) {
                    TaskID id = pair.first;
                    const TaskInfo& info = pair.second;
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.last_feed).count();
                    if (elapsed_ms > info.threshold_ms) {
                        bool dependency_ok = true;
                        auto dep_it = dependency_graph_.find(id);
                        if (dep_it != dependency_graph_.end()) {
                            for (TaskID dep : dep_it->second) {
                                auto it_dep = tasks_.find(dep);
                                if (it_dep != tasks_.end()) {
                                    auto dep_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it_dep->second.last_feed).count();
                                    if (dep_elapsed > it_dep->second.threshold_ms) {
                                        dependency_ok = false;
                                        std::cout << "[Watchdog] Dependency violation: Task " << id
                                        << " depends on task " << dep << "but it is stale.\n";
                                        break;
                                    }
                                }
                            }
                        }
                        if (!dependency_ok) {
                            std::cout << "[Watchdog] Task " << id << "feed not received in time (dependency check)!\n";
                        } else {
                            std::cout << "[Watchdog] Task " << id << "feed not received in time!\n";
                        }
                        // In the real system, this area has the reset or safety change mode over here.
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms_));
        }
    }
};

#endif // WATCHDOG_HPP
