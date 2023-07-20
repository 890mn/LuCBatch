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

namespace fs = std::filesystem;

const std::wstring output_png_folder = L"output_png";
const std::wstring output_mp4_folder = L"output_mp4";
const int max_file_size_kb = 600;

// 添加一个新函数，用于判断目录是否是受限制的目录
bool isRestrictedDirectory(const std::wstring& directory) {
    static const std::vector<std::wstring> restrictedDirectories = {
        L"$Recycle.Bin",
        L"$Windows.~WS",
        // 添加其他受限制的目录（如果有的话）
    };

    for (const auto& restrictedDir : restrictedDirectories) {
        if (directory.find(restrictedDir) != std::wstring::npos) {
            return true;
        }
    }

    return false;
}

// 添加一个新函数，用于获取系统的驱动器列表
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
    bool found = false;

    std::wcout << L"Searching for N0vaDesktop folder in drive " << driveLetter << L"..." << std::endl;

    // 如果不指定盘符或指定盘符为C盘，则只搜索C:/Program Files目录
    if (driveLetter.empty() || driveLetter == L"C") {
        fs::path programFilesDir = L"C:\\Program Files";
        if (fs::exists(programFilesDir)) {
            if (fs::is_directory(programFilesDir / L"N0vaDesktop")) {
                rootPath = (programFilesDir / L"N0vaDesktop\\N0vaDesktopCache\\game").wstring();
                found = true;
            }
        }
        if (driveLetter == L"C") {
            return found;
        }
    }

    // 其他盘符的搜索与之前的代码保持不变
    std::queue<fs::path> foldersQueue;
    foldersQueue.push(root);

    while (!foldersQueue.empty() && !found) {
        fs::path currentPath = foldersQueue.front();
        foldersQueue.pop();

        for (const auto& entry : fs::directory_iterator(currentPath)) {
            if (fs::is_directory(entry)) {
                // 排除受限制的目录，如系统目录和Windows目录
                if (entry.path().filename() == L"Windows" || entry.path().filename() == L"System Volume Information") {
                    continue;
                }

                if (entry.path().filename() == L"N0vaDesktop") {
                    rootPath = (entry.path() / L"N0vaDesktopCache\\game").wstring();
                    found = true;
                    break;
                } else {
                    foldersQueue.push(entry.path());
                    std::wcout << L"Searching in path " << entry.path().filename() << L"..." << std::endl;
                }
            }
        }

        // 如果找到N0vaDesktop文件夹，则不再继续搜索其他目录
        if (found) {
            while (!foldersQueue.empty()) {
                foldersQueue.pop();
            }
        }
    }

    if (found) {
        std::wcout << L"N0vaDesktop folder found in drive " << driveLetter << L"." << std::endl;
    }

    return found;
}


void extractFiles(const std::wstring& rootPath) {
    int pngCount = 0;
    int mp4Count = 0;

    fs::create_directory(output_png_folder);
    fs::create_directory(output_mp4_folder);

    // Set output stream back to binary mode for correct file reading
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stderr), _O_U8TEXT);

    for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
        if (!fs::is_regular_file(entry)) {
            continue;
        }

        if (fs::file_size(entry) < max_file_size_kb * 1024) {
            continue; // Skip files smaller than 600KB
        }

        std::ifstream inputFile(entry.path(), std::ios::binary); // Use std::ifstream instead of std::wifstream
        if (!inputFile) {
            std::wcerr << L"Error opening file: " << entry.path() << std::endl;
            continue;
        }

        std::vector<unsigned char> headerBytes(8);
        inputFile.read(reinterpret_cast<char*>(headerBytes.data()), headerBytes.size()); // Use char* instead of wchar_t*

        if (inputFile.gcount() < 8) {
            continue; // File is smaller than 8 bytes, skip it.
        }

        if (headerBytes[0] == 0x89 && headerBytes[1] == 0x50 && headerBytes[2] == 0x4E && headerBytes[3] == 0x47) {
            if (entry.path().extension() == L".ndf") {
                // Rename and move to output_png_folder
                fs::path outputPath = fs::path(output_png_folder) / (std::to_wstring(++pngCount) + L"_novaPic.png");
                fs::copy_file(entry.path(), outputPath, fs::copy_options::overwrite_existing);
                std::wcout << L"Extracted PNG: " << entry.path() << L" -> " << outputPath << std::endl;
            }
        } else if ((headerBytes[0] == 0x00 && headerBytes[1] == 0x00 && headerBytes[2] == 0x00 && headerBytes[3] == 0x00 &&
                    (headerBytes[5] == 0x18 || headerBytes[5] == 0x20) &&
                    headerBytes[4] == 0x00 && headerBytes[6] == 0x66 && headerBytes[7] == 0x74)) {
            if (entry.path().extension() == L".ndf") {
                // Rename and move to output_mp4_folder
                fs::path outputPath = fs::path(output_mp4_folder) / (std::to_wstring(++mp4Count) + L"_novaMov.mp4");
                std::ofstream outputFile(outputPath, std::ios::binary); // Use std::ofstream instead of std::wofstream

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

    // Reset output stream to wide character mode
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
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
    } else {
        foundN0vaDesktop = findAndExtractN0vaDesktop(driveLetter, rootPath);
    }

    if (!foundN0vaDesktop) {
        std::wcerr << L"Error: N0vaDesktop folder not found on any drive." << std::endl;
        return 1;
    }

    if (!rootPath.empty()) {
        extractFiles(rootPath);
    } else {
        std::wcerr << L"Error: N0vaDesktop folder not found on the provided drive letter." << std::endl;
        return 1;
    }

    std::wcout << L"Extraction completed." << std::endl;
    return 0;
}