#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <iomanip>
#include <queue>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <locale>
#include <unordered_map> 
#include <thread>        
#include <chrono>
#include <random> 
namespace fs = std::filesystem;

int pngCount = 0;
int mp4Count = 0;
std::wstring response;
const std::wstring output_png_folder = L"output_png";
const std::wstring output_mp4_folder = L"output_mp4";
const int max_file_size_kb = 600;

// 获取本次随机数种子
std::wstring getRandomSeed() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(10000, 99999);
    int randomSeed = distribution(gen);
    return std::to_wstring(randomSeed);
}
auto seed = getRandomSeed();

// 将wstring转换为string
std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// 判断目录是否是受限制的目录
bool isRestrictedDirectory(const std::wstring& directory) {
    static const std::vector<std::wstring> restrictedDirectories = {
        L"Windows",
        L"System Volume Information",
        // 添加其他受限制的目录（如果有的话）
    };
    for (const auto& restrictedDir : restrictedDirectories) {
        if (_wcsicmp(directory.c_str(), restrictedDir.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

// 获取系统的驱动器列表
std::vector<std::wstring> getSystemDriveLetters() {
    DWORD drivesMask = GetLogicalDrives();
    std::vector<std::wstring> drives;

    for (wchar_t drive = L'A'; drive <= L'Z'; ++drive) {
        if (drivesMask & 1) {
            std::wstring drivePath = std::wstring(1, drive) + L":\\";
            if (!isRestrictedDirectory(drivePath)) {
                drives.push_back(std::wstring(1, drive));
            }
        }
        drivesMask >>= 1;
    }
    return drives;
}

bool findAndExtractN0vaDesktop(const std::wstring& driveLetter, std::wstring& rootPath) {
    std::wstring root = driveLetter + L":\\";
    std::wcout << L"Searching for N0vaDesktop folder in drive " << driveLetter << L"..." << std::endl;

    // 如果不指定盘符或指定盘符为C盘，则只搜索C:/Program Files目录
    if (driveLetter.empty() || driveLetter == L"C") {
        fs::path programFilesDir = L"C:\\Program Files";
        if (fs::exists(programFilesDir)) {
            fs::path n0vaDesktopDir = programFilesDir / L"N0vaDesktop";
            if (fs::exists(n0vaDesktopDir)) {
                fs::path cacheDir = n0vaDesktopDir / L"N0vaDesktopCache\\game";
                if (fs::exists(cacheDir)) {
                    rootPath = cacheDir.wstring();
                    return true;
                }
            }
        }
        if (driveLetter == L"C") {
            return false;
        }
    }

    // 其他盘符的搜索与之前的代码保持不变
    std::queue<fs::path> foldersQueue;
    foldersQueue.push(root);

    while (!foldersQueue.empty()) {
        fs::path currentPath = foldersQueue.front();
        foldersQueue.pop();
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            if (fs::is_directory(entry)) {
                // 排除受限制的目录，如系统目录和Windows目录
                if (isRestrictedDirectory(entry.path().filename().wstring())) {
                    continue;
                }
                if (entry.path().filename() == L"N0vaDesktop") {
                    fs::path cacheDir = entry.path() / L"N0vaDesktopCache\\game";
                    if (fs::exists(cacheDir)) {
                        rootPath = cacheDir.wstring();
                        std::wcout << L"N0vaDesktop folder found in drive " << driveLetter << L"." << std::endl;
                        return true;
                    }
                } else {
                    foldersQueue.push(entry.path());
                    std::wcout << L"Searching in path " << entry.path().filename() << L"..." << std::endl;
                }
            }
        }
    }
    std::wcout << L"N0vaDesktop folder not found in drive " << driveLetter << L"." << std::endl;
    return false;
}

void extractFiles(const std::wstring& rootPath) {
    fs::create_directory(output_png_folder);
    fs::create_directory(output_mp4_folder);

    // Set output stream back to binary mode for correct file reading
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stderr), _O_U8TEXT);

    for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
        if (!fs::is_regular_file(entry)) continue;
        if (fs::file_size(entry) < max_file_size_kb * 1024) continue; // Skip files smaller than 600KB

        std::ifstream inputFile(wstringToString(entry.path().wstring()), std::ios::binary);
        if (!inputFile) {
            std::wcerr << L"Error opening file: " << entry.path() << std::endl;
            continue;
        }
        std::vector<unsigned char> headerBytes(8);
        inputFile.read(reinterpret_cast<char*>(headerBytes.data()), headerBytes.size());

        if (inputFile.gcount() < 8) continue; // File is smaller than 8 bytes, skip it.
        if (headerBytes[0] == 0x89 && headerBytes[1] == 0x50 && headerBytes[2] == 0x4E && headerBytes[3] == 0x47) {
            if (entry.path().extension() == L".ndf") {
                // Rename and move to output_png_folder
                fs::path outputPath = fs::path(output_png_folder) / (seed + L"_" + std::to_wstring(++pngCount) + L"_novaPic.png");
                fs::copy_file(entry.path(), outputPath, fs::copy_options::overwrite_existing);
                std::wcout << L"Extracted PNG: " << entry.path() << L" -> " << outputPath << std::endl;
            }
        } else if(headerBytes[0] == 0xFF && headerBytes[1] == 0xD8 && headerBytes[2] == 0xFF && (headerBytes[3] == 0xE0 || headerBytes[3] == 0xE1)){
            if (entry.path().extension() == L".ndf") {
                // Rename and move to output_png_folder
                fs::path outputPath = fs::path(output_png_folder) / (seed + L"_" + std::to_wstring(++pngCount) + L"_novaPic.png");
                fs::copy_file(entry.path(), outputPath, fs::copy_options::overwrite_existing);
                std::wcout << L"Extracted PNG: " << entry.path() << L" -> " << outputPath << std::endl;
            }
        } 
        else if ((headerBytes[0] == 0x00 && headerBytes[1] == 0x00 && headerBytes[2] == 0x00 && headerBytes[3] == 0x00 &&
                    (headerBytes[5] == 0x18 || headerBytes[5] == 0x20 || headerBytes[5] == 0x2C) &&
                    headerBytes[4] == 0x00 && headerBytes[6] == 0x66 && headerBytes[7] == 0x74)) {
            if (entry.path().extension() == L".ndf") {
                // Rename and move to output_mp4_folder
                fs::path outputPath = fs::path(output_mp4_folder) / (seed + L"_" + std::to_wstring(++mp4Count) + L"_novaMov.mp4");
                std::ofstream outputFile(outputPath, std::ios::binary);

                // Skip first 2 bytes and write the rest
                char buffer[1024];
                inputFile.seekg(2, std::ios::beg);
                while (!inputFile.eof()) {
                    inputFile.read(buffer, sizeof(buffer));
                    outputFile.write(buffer, inputFile.gcount());
                }

                std::wcout << L"Extracted MP4: " << entry.path() << L" -> " << outputPath << std::endl;
            }
        }
        std::wcout << L"Extraction completed." << std::endl;
        inputFile.close();
    }

    // Reset output stream to wide character mode
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
}

void extractNewFiles(const std::wstring& rootPath, std::unordered_map<std::wstring, long long unsigned int>& oldFileSizes) {
    int flag = 0;
    fs::create_directory(output_png_folder);
    fs::create_directory(output_mp4_folder);

    int pngCount = 0; // Add this line for pngCount initialization
    int mp4Count = 0; // Add this line for mp4Count initialization

    // Set output stream back to binary mode for correct file reading
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stderr), _O_U8TEXT);

    std::wcout << L"The Programme is running with dynamic screen-refresh." << std::endl;
    std::wcout << L"You can leave whenever just by you choose [ctrl-c] or [quit by your like]" << std::endl;
    while (true) {
        for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (!fs::is_regular_file(entry)) continue;
            if (fs::file_size(entry) < max_file_size_kb * 1024) continue; // Skip files smaller than 600KB

            const std::wstring filePath = entry.path().wstring();
            if (oldFileSizes.find(filePath) == oldFileSizes.end()) {
                std::ifstream inputFile(wstringToString(filePath), std::ios::binary); // Convert wstring to string
                if (!inputFile) {
                    std::wcerr << L"Error opening file: " << entry.path() << std::endl;
                    continue;
                }
                std::vector<unsigned char> headerBytes(8);
                inputFile.read(reinterpret_cast<char*>(headerBytes.data()), headerBytes.size());

                if (inputFile.gcount() < 8) continue; // File is smaller than 8 bytes, skip it.
                if (headerBytes[0] == 0x89 && headerBytes[1] == 0x50 && headerBytes[2] == 0x4E && headerBytes[3] == 0x47) {
                    // ... (extract PNG file code)
                    flag = 1;
                    if (entry.path().extension() == L".ndf") {
                        // Rename and move to output_png_folder
                        fs::path outputPath = fs::path(output_png_folder) / (seed + L"_" + std::to_wstring(++pngCount) + L"_novaPic.png");
                        fs::copy_file(entry.path(), outputPath, fs::copy_options::overwrite_existing);
                        std::wcout << L"Extracted PNG: " << entry.path() << L" -> " << outputPath << std::endl;
                    }
                } else if(headerBytes[0] == 0xFF && headerBytes[1] == 0xD8 && headerBytes[2] == 0xFF && (headerBytes[3] == 0xE0 || headerBytes[3] == 0xE1)){
                    if (entry.path().extension() == L".ndf") {
                        // Rename and move to output_png_folder
                        fs::path outputPath = fs::path(output_png_folder) / (seed + L"_" + std::to_wstring(++pngCount) + L"_novaPic.png");
                        fs::copy_file(entry.path(), outputPath, fs::copy_options::overwrite_existing);
                        std::wcout << L"Extracted PNG: " << entry.path() << L" -> " << outputPath << std::endl;
                    }
                }
                else if ((headerBytes[0] == 0x00 && headerBytes[1] == 0x00 && headerBytes[2] == 0x00 && headerBytes[3] == 0x00 &&
                            (headerBytes[5] == 0x18 || headerBytes[5] == 0x20 || headerBytes[5] == 0x2C) &&
                            headerBytes[4] == 0x00 && headerBytes[6] == 0x66 && headerBytes[7] == 0x74)) {
                    // ... (extract MP4 file code)
                    flag = 1;
                    if (entry.path().extension() == L".ndf") {
                        // Rename and move to output_mp4_folder
                        fs::path outputPath = fs::path(output_mp4_folder) / (seed + L"_" + std::to_wstring(++mp4Count) + L"_novaMov.mp4");
                        std::ofstream outputFile(outputPath, std::ios::binary);

                        // Skip first 2 bytes and write the rest
                        char buffer[1024];
                        inputFile.seekg(2, std::ios::beg);
                        while (!inputFile.eof()) {
                            inputFile.read(buffer, sizeof(buffer));
                            outputFile.write(buffer, inputFile.gcount());
                        }

                        std::wcout << L"Extracted MP4: " << entry.path() << L" -> " << outputPath << std::endl;
                    }
                }
                inputFile.close();
            }
        }

        // Get the current file sizes for comparison in the next iteration
        oldFileSizes.clear();
        for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (fs::is_regular_file(entry)) {
                const std::wstring filePath = entry.path().wstring();
                oldFileSizes[filePath] = fs::file_size(entry);
            }
        }

        // Delay before the next iteration
        if(flag) std::this_thread::sleep_for(std::chrono::seconds(1));
        else std::this_thread::sleep_for(std::chrono::seconds(2));

    }
}

int main() {
    // Set output stream to wide character mode for Unicode support on Windows
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    std::wcout << L"Do you know the drive letter for the N0vaDesktop folder? (Enter the drive letter or leave empty): ";
    std::wstring driveLetter;
    std::getline(std::wcin, driveLetter);

    std::wstring rootPath;
    bool foundN0vaDesktop = false;

    if (driveLetter.empty()) {
        std::vector<std::wstring> drives = getSystemDriveLetters();
        for (const std::wstring& drive : drives) {
            if (findAndExtractN0vaDesktop(drive, rootPath)) {
                foundN0vaDesktop = true;
                break;
            }
        }
    } else foundN0vaDesktop = findAndExtractN0vaDesktop(driveLetter, rootPath);

    if (!foundN0vaDesktop) {
        std::wcerr << L"Error: N0vaDesktop folder not found on any drive." << std::endl;
        return 1;
    }

    if (!rootPath.empty()) {
        std::wcout << L"Do you want to start downloading files? (y/n): ";
        std::getline(std::wcin, response);

        if (response == L"y") {
            std::wcout << L"Current Random Seed: " << seed << std::endl;

            // Create an unordered_map to hold old file sizes
            std::unordered_map<std::wstring, uintmax_t> oldFileSizes;
            
            // Initial file scan to get old file sizes
            for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
                if (fs::is_regular_file(entry)) {
                    oldFileSizes[entry.path().wstring()] = fs::file_size(entry);
                }
            }

            // Extract new files and monitor for changes
            extractNewFiles(rootPath, oldFileSizes);
        } else {
            std::wcout << L"Extraction canceled. No files will be downloaded." << std::endl;
        }
    } else {
        std::wcerr << L"Error: N0vaDesktop folder not found on the provided drive letter." << std::endl;
        return 1;
    }
    return 0;
}