#ifndef GAMEENGINE_FILE_H
#define GAMEENGINE_FILE_H

#include <iostream>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

std::vector<std::string> getFilenamesInDirectory(const std::string& folderPath) {
    std::vector<std::string> filenames;

    try {
        // Check if the folder exists
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
            throw std::runtime_error("The specified path does not exist or is not a directory.");
        }

        // Iterate through the folder and collect filenames
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            filenames.push_back(entry.path().filename().string());
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "General error: " << e.what() << '\n';
    }

    return filenames;
}

#endif //GAMEENGINE_FILE_H
