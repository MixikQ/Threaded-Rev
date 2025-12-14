#pragma once
#include "ThreadedSafeQueue.h"
#include "ConfigManager.h"
#include <vector>
#include <thread>
#include <atomic>
#include <filesystem>

namespace fs = std::filesystem;

class Producer {
private:
    ThreadedSafeQueue& queue;
    const ConfigManager& config;
    std::atomic<bool> running;
    std::thread producerThread;

    void processFolder();
    void exploreDirectory(const fs::path& directory);

public:
    Producer(ThreadedSafeQueue& q, const ConfigManager& cfg);
    ~Producer();

    void start();
    void stop();
    void waitForCompletion();
    bool isRunning() const { return running; }
};
