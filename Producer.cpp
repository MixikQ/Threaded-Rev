#include "Producer.h"
#include <iostream>
#include <chrono>

Producer::Producer(ThreadedSafeQueue& q, const ConfigManager& cfg)
    : queue(q), config(cfg), running(false) {
    std::cout << "[Producer] Producer created\n";
}

Producer::~Producer() {
    stop();
    waitForCompletion();
    std::cout << "[Producer] Producer destroyed\n";
}

void Producer::start() {
    if (running.exchange(true)) {
        std::cout << "[Producer] Producer is already running\n";
        return;
    }

    producerThread = std::thread(&Producer::processFolder, this);
    std::cout << "[Producer] Producer started\n";
}

void Producer::stop() {
    if (!running.exchange(false)) {
        return;
    }
    std::cout << "[Producer] Producer stop requested\n";
    queue.stop();
}

void Producer::waitForCompletion() {
    if (producerThread.joinable()) {
        if (std::this_thread::get_id() == producerThread.get_id()) {
            std::cerr << "[Producer] ERROR: Deadlock! Cannot join self\n";
            return;
        }
        producerThread.join();
        std::cout << "[Producer] Producer thread joined\n";
    }
}

void Producer::exploreDirectory(const fs::path& directory) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (!running.load()) {
                std::cout << "[Producer] Stopping directory exploration\n";
                return;
            }

            if (fs::is_regular_file(entry)) {
                std::string filePath = entry.path().string();

                if (config.isImageFile(filePath)) {
                    std::string outputPath = config.createOutputPath(filePath);

                    if (!outputPath.empty()) {
                        Task task(filePath, outputPath);
                        queue.push(task);

                    }
                    else {
                        std::cerr << "[Producer] Failed to create output path for: " << filePath << "\n";
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "[Producer] Filesystem error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[Producer] Error exploring directory: " << e.what() << "\n";
    }
}

void Producer::processFolder() {
    std::cout << "[Producer] Starting to process folder: " 
              << config.getInputFolder() << "\n";

    queue.setProducerCount(1);
    
    exploreDirectory(config.getInputFolder());
    
    if (running.load()) {
        std::cout << "[Producer] Finished processing folder\n";
    } else {
        std::cout << "[Producer] Producer stopped by request\n";
    }
    queue.addPoisonPill();
    queue.producerFinished();
    running = false;
}