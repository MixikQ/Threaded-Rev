#include "ThreadedSafeQueue.h"

ThreadedSafeQueue::ThreadedSafeQueue() 
    : stopFlag(false), remainingProducers(0) {
    std::cout << "[Queue] Thread-safe queue initialized\n";
}

ThreadedSafeQueue::~ThreadedSafeQueue() {
    stop();
    std::cout << "[Queue] Queue destroyed\n";
}

void ThreadedSafeQueue::push(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(task);
    std::cout << "[Queue] Task added: " << task.inputPath 
              << " (queue size: " << queue.size() << ")\n";
    condition.notify_one();
}

bool ThreadedSafeQueue::pop(Task& task) {
    std::unique_lock<std::mutex> lock(mutex);
    
    condition.wait(lock, [this]() { 
        return !queue.empty() || remainingProducers <= 0; 
    });
    
    if (queue.empty()) {
        return false;
    }

    task = queue.front();
    queue.pop();
    
    if (task.isPoisonPill()) {
        return true;
    }

    std::cout << "[Queue] Task taken: " << task.inputPath 
              << " (queue size: " << queue.size() << ")\n";
    
    return true;
}

size_t ThreadedSafeQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.size();
}

void ThreadedSafeQueue::setProducerCount(int count) {
    remainingProducers = count;
    std::cout << "[Queue] Producer count set to: " << count << "\n";
}

void ThreadedSafeQueue::producerFinished() {
    int remaining = --remainingProducers;
    std::cout << "[Queue] Producer finished. Remaining: " << remaining << "\n";
    
    if (remaining == 0) {
        condition.notify_all();
    }
}

void ThreadedSafeQueue::addPoisonPill() {
    Task poisonPill("POISON_PILL", "");
    push(poisonPill);
    std::cout << "[Queue] Poison pill added\n";
}

void ThreadedSafeQueue::stop() {
    stopFlag = true;
    condition.notify_all();
    std::cout << "[Queue] Queue stopped\n";
}

bool ThreadedSafeQueue::isFinished() const {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty() && remainingProducers == 0;
}
