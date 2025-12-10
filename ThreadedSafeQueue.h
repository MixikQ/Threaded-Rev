#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <iostream>

struct Task {
    std::string inputPath;
    std::string outputPath;
    
    Task(const std::string& input = "", const std::string& output = "")
        : inputPath(input), outputPath(output) {}
    
    bool isPoisonPill() const {
        return inputPath == "POISON_PILL";
    }
};

class ThreadedSafeQueue {
private:
    std::queue<Task> queue;
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    std::atomic<int> remainingProducers;
    
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

public:
    ThreadedSafeQueue();
    ~ThreadedSafeQueue();
    
    void push(const Task& task);
    bool pop(Task& task);
    size_t size() const;
    
    void setProducerCount(int count);
    void producerFinished();
    void addPoisonPill();
    void stop();
    bool isFinished() const;
};
