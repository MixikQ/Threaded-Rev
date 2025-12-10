#include "ConfigManager.h"
#include <iostream>
#include <algorithm>

ConfigManager::ConfigManager() 
    : inputFolder("./input"), outputFolder("./output"), numThreads(4) {
    supportedExtensions = {".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".gif"};
}

bool ConfigManager::loadFromCommandLine(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: Not enough arguments\n";
        printUsage(argv[0]);
        return false;
    }
    
    inputFolder = argv[1];
    outputFolder = argv[2];
    
    if (argc >= 4) {
        try {
            numThreads = std::stoi(argv[3]);
            if (numThreads < 1) numThreads = 4;
        } catch (...) {
            numThreads = 4;
        }
    }
    
    try {
        if (!fs::exists(outputFolder)) {
            fs::create_directories(outputFolder);
            std::cout << "[Config] Created output folder: " << outputFolder << "\n";
        }
    } catch (...) {
        return false;
    }
    
    return validate();
}

bool ConfigManager::validate() const {
    if (!fs::exists(inputFolder) || !fs::is_directory(inputFolder)) {
        std::cerr << "Error: Input folder does not exist: " << inputFolder << "\n";
        return false;
    }
    
    if (numThreads < 1) {
        std::cerr << "Error: Number of threads must be positive\n";
        return false;
    }
    
    std::cout << "[Config] Configuration validated\n";
    return true;
}

bool ConfigManager::isImageFile(const std::string& filePath) const {
    std::string extension = fs::path(filePath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    for (const auto& ext : supportedExtensions) {
        if (extension == ext) return true;
    }
    return false;
}

std::string ConfigManager::createOutputPath(const std::string& inputPath) const {
    try {
        fs::path relativePath = fs::relative(inputPath, inputFolder);
        fs::path outputPath = fs::path(outputFolder) / relativePath;
        fs::create_directories(outputPath.parent_path());
        return outputPath.string();
    } catch (...) {
        return "";
    }
}

void ConfigManager::printConfig() const {
    std::cout << "\n=== Configuration ===\n";
    std::cout << "Input Folder: " << inputFolder << "\n";
    std::cout << "Output Folder: " << outputFolder << "\n";
    std::cout << "Threads: " << numThreads << "\n";
    std::cout << "Extensions: ";
    for (const auto& ext : supportedExtensions) std::cout << ext << " ";
    std::cout << "\n====================\n\n";
}

void ConfigManager::printUsage(const std::string& programName) {
    std::cout << "\nUsage: " << programName << " <input_folder> <output_folder> [threads]\n";
    std::cout << "Example: " << programName << " ./images ./processed 4\n";
}
