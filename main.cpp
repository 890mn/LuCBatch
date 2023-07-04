#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>

std::vector<std::string> filenames;

void GetDir() {
    std::string directoryPath = "I:\\study data\\C_code\\LuCBatch";  // 替换为你的目录路径
    
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)){
        if (entry.is_regular_file() && entry.path().extension() == ".ndf") {  // 只处理常规文件
            std::string filename = entry.path().filename().string();
            filenames.push_back(filename);
        }
    }

    // 打印文件名
    for (const std::string& filename : filenames){
        std::cout << filename << std::endl;
    }
}

void BitDelete() {
    std::ifstream fin;
    fin.open(filenames, std::ios::binary);
    if (!fin.is_open()) {
        std::cout << "Open failed." << std::endl;
        return;
    }
    std::ofstream tmp;
    tmp.open("tmp" + filenames +".mp4", std::ios::binary);
    if (!tmp.is_open()) {
        std::cout << "Failed to create tmp.mp4" << std::endl;
        fin.close();
        return;
    }
    fin.seekg(2);
    char c;
    while (fin.get(c))
        tmp.put(c);
    fin.close();
    tmp.close();
}

int main() {
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    GetDir();
    BitDelete();
    return 0;
}
