#include "ConfigManager.h"
#include "Producer.h"
#include "Consumer.h"
#include "ThreadedSafeQueue.h"
#include <iostream>
#include <vector>
#include <memory>
#include <csignal>
#include <atomic>

std::atomic<bool> g_stop_requested{false};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[Main] Signal received. Stopping...\n";
        g_stop_requested.store(true);
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    std::cout << "=== Threaded Image Processor ===\n";
    
    try {
        ConfigManager config;
        if (!config.loadFromCommandLine(argc, argv)) {
            return 1;
        }
        config.printConfig();
        ThreadedSafeQueue queue;
        Producer producer(queue, config);
        
        std::vector<std::unique_ptr<Consumer>> consumers;
        int numThreads = config.getNumThreads();
        
        std::cout << "[Main] Creating " << numThreads << " consumer threads\n";
        for (int i = 0; i < numThreads; ++i) {
            consumers.emplace_back(std::make_unique<Consumer>(i, queue));
        }
        
        std::cout << "[Main] Starting all components...\n";
        producer.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for (auto& consumer : consumers) {
            consumer->start();
        }
        
        std::cout << "[Main] Processing started. Press Ctrl+C to stop.\n";
        
        while (!g_stop_requested.load()) {
            if (!producer.isRunning()) {
                if (queue.isFinished()) {
                    std::cout << "[Main] All tasks processed. Queue is finished.\n";
                    break;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            static int progressCounter = 0;
            if (++progressCounter % 10 == 0) {
                size_t queueSize = queue.size();
                std::cout << "[Main] Queue size: " << queueSize 
                          << ", Producer running: " << (producer.isRunning() ? "yes" : "no")
                          << "\n";

                for (const auto& consumer : consumers) {
                    std::cout << "  Consumer " << consumer->getId() 
                              << ": " << consumer->getProcessedCount() << " tasks\n";
                }
            }
        }
        if (g_stop_requested.load()) {
            std::cout << "\n[Main] Stop requested. Stopping all components...\n";
            producer.stop();
            
            for (size_t i = 0; i < consumers.size(); ++i) {
                queue.addPoisonPill();
            }
            
            for (auto& consumer : consumers) {
                consumer->stop();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            queue.stop();
        }
        
        std::cout << "[Main] Waiting for producer to finish...\n";
        producer.waitForCompletion();
        
        std::cout << "[Main] Waiting for consumers to finish...\n";
        for (auto& consumer : consumers) {
            consumer->waitForCompletion();
        }
        
        std::cout << "\n=== PROCESSING COMPLETE ===\n";
        
        size_t totalProcessed = 0;
        for (const auto& consumer : consumers) {
            int count = consumer->getProcessedCount();
            totalProcessed += count;
            std::cout << "Consumer " << consumer->getId() 
                      << ": " << count << " tasks\n";
        }
        
        std::cout << "\nTotal images processed: " << totalProcessed << "\n";
        std::cout << "Final queue size: " << queue.size() << "\n";
        
        if (g_stop_requested.load()) {
            std::cout << "Processing was interrupted by user.\n";
        } else {
            std::cout << "Processing completed successfully.\n";
        }
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Fatal error: " << e.what() << "\n";
        return 1;
    }
}