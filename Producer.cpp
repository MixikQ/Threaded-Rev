#include "Producer.h"
#include <iostream>
#include <chrono>

Producer::Producer(ThreadedSafeQueue& q, const ConfigManager& cfg)
    : queue(q), config(cfg), running(false) {
    std::cout << "[Producer] Producer created\n";
}

Producer::~Producer() {
    stop();
    if (producerThread.joinable()) {
        producerThread.join();
    }
    std::cout << "[Producer] Producer destroyed\n";
}

void Producer::start() {
    if (running) {
        std::cout << "[Producer] Producer is already running\n";
        return;
    }

    running = true;
    producerThread = std::thread(&Producer::processFolder, this);
    std::cout << "[Producer] Producer started\n";
}

void Producer::stop() {
    running = false;
    std::cout << "[Producer] Producer stop requested\n";
}

void Producer::waitForCompletion() {
    if (producerThread.joinable()) {
        producerThread.join();
        std::cout << "[Producer] Producer thread joined\n";
    }
}

void Producer::exploreDirectory(const fs::path& directory) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (!running) {
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

                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    std::cout << "[Producer] Starting to process folder: " << config.getInputFolder() << "\n";

    exploreDirectory(config.getInputFolder());

    if (running) {
        std::cout << "[Producer] Finished processing folder\n";
        std::cout << "[Producer] Producer finished work\n";
    }
    else {
        std::cout << "[Producer] Producer stopped by request\n";
    }

    queue.producerFinished();
    running = false;
}