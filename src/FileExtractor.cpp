#include "fileextractor.h"
#include <QDebug>
#include <queue>
#include <chrono>
#include <fstream>
#include <windows.h>

FileExtractor::FileExtractor(QObject *parent) : QObject(parent), stopListening(false) {
    rescanFolder();
}

FileExtractor::~FileExtractor() {
    stopListening = true;
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
}

void FileExtractor::rescanFolder() {
    std::wstring rootPath;
    if (findN0vaDesktop(rootPath)) {
        detectedFolder = QString::fromStdWString(rootPath);
        emit extractionLog("N0vaDesktop folder found: " + detectedFolder);
    } else {
        detectedFolder = "";
        emit extractionLog("N0vaDesktop folder not found.");
    }
}

QString FileExtractor::getShowFolder() {
    return showFolder;
}

QString FileExtractor::getDetectedFolder() {
    return detectedFolder;
}

void FileExtractor::startExtraction(const QString &folderPath) {
    std::wstring rootPath = folderPath.toStdWString();
    if (fs::exists(rootPath)) {
        emit extractionLog("Starting file monitoring...");

        initialFileSizes.clear();
        for (const auto &entry : fs::directory_iterator(rootPath)) {
            if (fs::is_regular_file(entry)) {
                initialFileSizes[entry.path().wstring()] = fs::file_size(entry);
            }
        }

        stopListening = false;
        listenerThread = std::thread(&FileExtractor::monitorNewFiles, this, rootPath);

    } else {
        emit extractionLog("Invalid folder path.");
    }
}

void FileExtractor::monitorNewFiles(const std::wstring &rootPath) {
    fs::path gameFolder = fs::path(rootPath);

    if (!fs::exists(gameFolder)) {
        emit extractionLog("Error: N0vaDesktopCache\\game folder does not exist.");
        return;
    }

    emit extractionLog("Monitoring folder: " + QString::fromStdWString(gameFolder.wstring()));

    while (!stopListening) {
        for (const auto &entry : fs::directory_iterator(gameFolder)) {
            if (fs::is_regular_file(entry)) {
                const std::wstring filePath = entry.path().wstring();

                if (filePath.find(L"_tmp") != std::wstring::npos) {
                    emit extractionLog("Skipping file: " + QString::fromStdWString(filePath) + " (still downloading)");
                    continue;
                }

                if (initialFileSizes.find(filePath) == initialFileSizes.end() && fs::file_size(entry) > 350 * 1024) {
                    initialFileSizes[filePath] = fs::file_size(entry);

                    std::thread extractorThread(&FileExtractor::extractFiles, this, filePath);
                    extractorThread.detach();

                    emit extractionLog("New file detected: " + QString::fromStdWString(filePath));
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    emit extractionLog("Monitoring stopped.");
}

void FileExtractor::extractFiles(const std::wstring &filePath) {
    fs::path output_png_folder = L"output_png";
    fs::path output_mp4_folder = L"output_mp4";
    fs::create_directory(output_png_folder);
    fs::create_directory(output_mp4_folder);

    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile.is_open()) {
        emit extractionLog("Failed to open file: " + QString::fromStdWString(filePath));
        return;
    }

    inputFile.seekg(0, std::ios::end);
    std::streamsize fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    std::vector<unsigned char> headerBytes(8);
    inputFile.read(reinterpret_cast<char*>(headerBytes.data()), headerBytes.size());

    if (inputFile.gcount() < 8) {
        emit extractionLog("File too small to process: " + QString::fromStdWString(filePath));
        return;
    }

    fs::path outputPath;
    emit progressUpdate(0);

    if ((headerBytes[0] == 0x89 && headerBytes[1] == 0x50 && headerBytes[2] == 0x4E && headerBytes[3] == 0x47) ||
        (headerBytes[0] == 0xFF && headerBytes[1] == 0xD8 && headerBytes[2] == 0xFF && headerBytes[3] == 0xE2)) {
        outputPath = output_png_folder / (fs::path(filePath).stem().wstring() + L".png");
        fs::copy(filePath, outputPath, fs::copy_options::overwrite_existing);

        emit extractionLog("Extracted PNG: " + QString::fromStdWString(outputPath.wstring()));
    }
    else {
        outputPath = output_mp4_folder / (fs::path(filePath).stem().wstring() + L".mp4");
        std::ofstream outputFile(outputPath, std::ios::binary);
        if (!outputFile.is_open()) {
            emit extractionLog("Failed to open output file: " + QString::fromStdWString(outputPath.wstring()));
            return;
        }

        inputFile.seekg(2, std::ios::beg);
        char buffer[1024];
        std::streamsize totalBytesRead = 0;
        while (!inputFile.eof()) {
            inputFile.read(buffer, sizeof(buffer));
            std::streamsize bytesRead = inputFile.gcount();
            outputFile.write(buffer, bytesRead);
            totalBytesRead += bytesRead;

            int progress = static_cast<int>((totalBytesRead * 100) / fileSize);
            emit progressUpdate(progress);
        }

        emit extractionLog("Extracted MP4: " + QString::fromStdWString(outputPath.wstring()));
        outputFile.close();
    }

    inputFile.close();
    emit extractionCompleted();
}

bool FileExtractor::findN0vaDesktop(std::wstring &rootPath) {
    DWORD drivesMask = GetLogicalDrives();
    for (wchar_t drive = L'C'; drive <= L'Z'; ++drive) {
        if (drivesMask & (1 << (drive - L'A'))) {
            std::wstring drivePath = std::wstring(1, drive) + L":\\";
            fs::path searchPath = drivePath;
            std::queue<fs::path> foldersQueue;
            foldersQueue.push(searchPath);

            while (!foldersQueue.empty()) {
                fs::path currentPath = foldersQueue.front();
                foldersQueue.pop();

                try {
                    for (const auto& entry : fs::directory_iterator(currentPath)) {
                        if (fs::is_directory(entry)) {
                            if (entry.path().filename() == L"N0vaDesktop") {
                                showFolder = QString::fromStdWString(entry.path().wstring());
                                rootPath = (entry.path() / L"N0vaDesktopCache\\game").wstring();
                                return true;
                            }
                            foldersQueue.push(entry.path());
                        }
                    }
                } catch (const std::exception &e) {
                    emit extractionLog("Skipping inaccessible directory: " + QString::fromStdWString(currentPath.wstring()));
                }
            }
        }
    }
    return false;
}
