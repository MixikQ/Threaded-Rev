#include "Consumer.h"
#include <filesystem>
#include <chrono>


namespace fs = std::filesystem;

Consumer::Consumer(int id, ThreadedSafeQueue& queue)
    : id_(id), queue_(queue) {
    std::cout << "[Consumer " << id_ << "] Created\n";
}

Consumer::~Consumer() {
    stop();
    if (worker_.joinable()) {
        worker_.join();
    }
    std::cout << "[Consumer " << id_ << "] Destroyed\n";
}

void Consumer::start() {
    if (running_) {
        std::cout << "[Consumer " << id_ << "] Already running\n";
        return;
    }
    running_ = true;
    worker_ = std::thread(&Consumer::run, this);
    std::cout << "[Consumer " << id_ << "] Started\n";
}

void Consumer::stop() {
    running_ = false;
    std::cout << "[Consumer " << id_ << "] Stop requested\n";
}

void Consumer::waitForCompletion() {
    if (worker_.joinable()) {
        worker_.join();
        std::cout << "[Consumer " << id_ << "] Thread joined\n";
    }
}

void Consumer::run() {
    std::cout << "[Consumer " << id_ << "] Thread running\n";

    while (running_.load()) {
        Task task;

        // Получаем задачу из очереди
        if (!queue_.pop(task)) {
            std::cout << "[Consumer " << id_ << "] Queue finished\n";
            break;
        }

        if (task.isPoisonPill()) {
            std::cout << "[Consumer " << id_ << "] Received poison pill\n";
            break;
        }

        std::cout << "[Consumer " << id_ << "] Processing: " << task.inputPath << "\n";

        auto start = std::chrono::high_resolution_clock::now();

        if (processTask(task)) {
            processed_++;
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "[Consumer " << id_ << "] OK " << duration.count() << " ms\n";
        }
        else {
            std::cerr << "[Consumer " << id_ << "] FAILED\n";
        }
    }

    running_.exchange(false);
    std::cout << "[Consumer " << id_ << "] Finished (" << processed_ << " tasks)\n";
}

void Consumer::invertImage(cv::Mat& image) {
    if (image.type() == CV_8UC3) { 
        for (int i = 0; i < image.rows; ++i) {
            for (int j = 0; j < image.cols; ++j) {
                cv::Vec3b& pixel = image.at<cv::Vec3b>(i, j);
                pixel[0] = 255 - pixel[0]; 
                pixel[1] = 255 - pixel[1];  
                pixel[2] = 255 - pixel[2];
            }
        }
    }
    else if (image.type() == CV_8UC1) { 
        for (int i = 0; i < image.rows; ++i) {
            for (int j = 0; j < image.cols; ++j) {
                uchar& pixel = image.at<uchar>(i, j);
                pixel = 255 - pixel;
            }
        }
    }
}

bool Consumer::processTask(const Task& task) {
    if (!running_.load(std::memory_order_acquire)) {
        return false;
    }

    try {
        cv::Mat image = cv::imread(task.inputPath, cv::IMREAD_COLOR);
        if (image.empty()) {
            throw std::runtime_error("Cannot load image" + task.inputPath);
        }

        invertImage(image);

        fs::path outputPathObj(task.outputPath);
        if (!outputPathObj.parent_path().empty()) {
            fs::create_directories(outputPathObj.parent_path());
        }

        std::vector<int> compression_params;
        if (outputPathObj.extension() == ".jpg" || outputPathObj.extension() == ".jpeg") {
            compression_params = {cv::IMWRITE_JPEG_QUALITY, 95};
        }

        if (!cv::imwrite(task.outputPath, image)) {
            throw std::runtime_error("Failed to save");
        }

        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "[Consumer " << id_ << "] Error: " << e.what() << "\n";
        return false;
    }
}