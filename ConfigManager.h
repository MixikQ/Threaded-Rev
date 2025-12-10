#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class ConfigManager {
private:
    std::string inputFolder;
    std::string outputFolder;
    int numThreads;
    std::vector<std::string> supportedExtensions;
    
public:
    ConfigManager();
    
    bool loadFromCommandLine(int argc, char* argv[]);
    bool validate() const;
    bool isImageFile(const std::string& filePath) const;
    std::string createOutputPath(const std::string& inputPath) const;
    void printConfig() const;
    
    const std::string& getInputFolder() const { return inputFolder; }
    const std::string& getOutputFolder() const { return outputFolder; }
    int getNumThreads() const { return numThreads; }
    
    static void printUsage(const std::string& programName);
};
