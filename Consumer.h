#pragma once

#include <atomic>
#include <thread>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "ThreadedSafeQueue.h"

class Consumer {
private:
    int id_;                          
    std::thread worker_;              
    ThreadedSafeQueue& queue_;        
    std::atomic<bool> running_{ false }; 
    std::atomic<int> processed_{ 0 };   

    void run();                       
    bool processTask(const Task& task); 
    void invertImage(cv::Mat& image); 

public:
    Consumer(int id, ThreadedSafeQueue& queue); 
    ~Consumer();                                
    Consumer(const Consumer&) = delete;
    Consumer& operator=(const Consumer&) = delete;

    void start();
    void stop();
    void waitForCompletion();

    int getId() const { return id_; }
    int getProcessedCount() const { return processed_; }
    bool isRunning() const { return running_; }
};